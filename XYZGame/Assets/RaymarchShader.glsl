#type vertex
#version 450 core

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;


layout (std140, binding = 0) uniform Scene
{
	mat4 u_ViewProjection;
	mat4 u_InverseView;
	mat4 u_CameraFrustum;
	vec4 u_CameraPosition;
	vec4 u_LightDirection;
	vec4 u_LightColor;
};

layout(push_constant) uniform Transform
{
	mat4 Transform;
} u_Renderer;


struct VertexOutput
{
	vec3 Ray;
	vec3 RayOrigin;

	vec3 LightDirection;
	vec3 LightColor;
};

layout (location = 0) out VertexOutput v_Output;

void main()
{
    gl_Position = u_Renderer.Transform * vec4(a_Position.xy, 0.0, 1.0);
	int index = int(a_Position.z);

	vec4 tempRay = u_CameraFrustum[index];
	tempRay /= abs(tempRay.z);
	tempRay = u_InverseView * tempRay;

	v_Output.Ray = normalize(tempRay.xyz);
	v_Output.RayOrigin = u_CameraPosition.xyz;
	v_Output.LightDirection = u_LightDirection.xyz;
	v_Output.LightColor = u_LightColor.xyz;
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;


#define VOXEL_GRID_SIZE 5


struct VertexOutput
{
	vec3 Ray;
	vec3 RayOrigin;

	vec3 LightDirection;
	vec3 LightColor;
};

layout(location = 0) in VertexOutput v_Input;


layout(std430, binding = 3) buffer buffer_Voxels
{
	int Voxels[VOXEL_GRID_SIZE][VOXEL_GRID_SIZE][VOXEL_GRID_SIZE];
};

layout(push_constant) uniform Uniforms
{
	layout(offset = 64) 
	float MaxDistance;
	float VoxelSize;
} u_Uniforms;


float sdSphere(vec3 position, float radius)
{
	return length(position) - radius;
}

float sdBox(vec3 p, vec3 b)
{
	vec3 d = abs(p) - b;
	return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float distanceField(vec3 position, vec3 boxPosition)
{
	float dist = sdBox(position - boxPosition, vec3(u_Uniforms.VoxelSize));
	return dist;
}

vec3 getNormal(vec3 position, vec3 boxPosition)
{
	const vec3 offset = vec3(0.001, 0.0, 0.0);
	vec3 normal = vec3(
		distanceField(position + offset.xyy, boxPosition) - distanceField(position - offset.xyy, boxPosition),
		distanceField(position + offset.yxy, boxPosition) - distanceField(position - offset.yxy, boxPosition),
		distanceField(position + offset.yyx, boxPosition) - distanceField(position - offset.yyx, boxPosition)
	);
	return normalize(normal);
}

float maxcomp(in vec3 p) { return max(p.x, max(p.y, p.z)); }
double maxcomp(in dvec3 p) { return max(p.x, max(p.y, p.z)); }
float maxcomp(in vec2 p) { return max(p.x, p.y); }
double maxcomp(in dvec2 p) { return max(p.x, p.y); }

float mincomp(in vec3 p) { return min(p.x, min(p.y, p.z)); }
double mincomp(in dvec3 p) { return min(p.x, min(p.y, p.z)); }

const float eps = 0.0005;

vec4 rayMarchTest(vec3 startPoint, vec3 dir)
{
	// Start point in voxel space
	vec3 p0 = startPoint / u_Uniforms.VoxelSize;
	float endT = mincomp(max((vec3(u_Uniforms.VoxelSize) - p0) / dir, -p0 / dir));

	vec3 p0abs = (1 - step(0, dir)) * u_Uniforms.VoxelSize + sign(dir) * p0;
	vec3 dirAbs = abs(dir);

	float t = 0;
	while (t <= endT) 
	{
		// Next point to check
		vec3 p = p0 + dir * t;
		vec3 tmpIndex = vec3(p / u_Uniforms.VoxelSize);
		int voxel = Voxels[int(tmpIndex.x)][int(tmpIndex.y)][int(tmpIndex.z)];

		// Stop if voxel is solid
		if (voxel == 255)
		{
			return vec4(v_Input.LightColor, 1.0);
		}

		vec3 pAbs = p0abs + dirAbs * t;
		vec3 deltas = (1 - fract(p)) / dirAbs;
		t += max(mincomp(deltas), eps) + voxel;
	}

	return vec4(-dir, 1.0);
}

vec4 rayMarch2(vec3 rayOrig, vec3 rayDir)
{
	vec4 defaultColor = vec4(-rayDir, 1.0);
	const int maxIteration = 128;

	vec3 origin = rayOrig;
	vec3 direction = rayDir;
	ivec3 current_voxel = ivec3(floor(origin / u_Uniforms.VoxelSize));

	ivec3 step = ivec3(
		(direction.x > 0.0) ? 1 : -1,
		(direction.y > 0.0) ? 1 : -1,
		(direction.z > 0.0) ? 1 : -1
	);
	vec3 next_boundary = vec3(
		float((step.x > 0) ? current_voxel.x + 1 : current_voxel.x) * u_Uniforms.VoxelSize,
		float((step.y > 0) ? current_voxel.y + 1 : current_voxel.y) * u_Uniforms.VoxelSize,
		float((step.z > 0) ? current_voxel.z + 1 : current_voxel.z) * u_Uniforms.VoxelSize
	);

	vec3 t_max = (next_boundary - origin) / direction; // we will move along the axis with the smallest value
	vec3 t_delta = u_Uniforms.VoxelSize / direction * vec3(step);
	vec3 normal;
	int voxel = 0;
	int i = 0;
	do 
	{
		if (t_max.x < t_max.y && t_max.x < t_max.z) 
		{
			normal = vec3(float(-step.x), 0.0, 0.0);
			t_max.x += t_delta.x;
			current_voxel.x += step.x;
			//if (current_voxel.x == VOXEL_GRID_SIZE)
			//	return defaultColor;
		}
		else if (t_max.y < t_max.z) 
		{
			normal = vec3(0.0, float(-step.y), 0.0);
			t_max.y += t_delta.y;
			current_voxel.y += step.y;
			//if (current_voxel.y == VOXEL_GRID_SIZE)
			//	return defaultColor;
		}
		else 
		{
			normal = vec3(0.0, 0.0, float(-step.z));
			t_max.z += t_delta.z;
			current_voxel.z += step.z;
			//if (current_voxel.z == VOXEL_GRID_SIZE)
			//	return defaultColor;
		}

		voxel = Voxels[current_voxel.x][current_voxel.y][current_voxel.z];
		if (voxel != 0)
		{
			float light = dot(-v_Input.LightDirection, normal);
			vec3 color = v_Input.LightColor * light;
			return vec4(color, 1.0);
		}
		i += 1;
	} 
	while (voxel == 0 && i < maxIteration);
	
	return defaultColor;
}

vec4 rayMarch(vec3 rayOrig, vec3 rayDir)
{
	float hitTreshold = 0.01;
	for (int voxelX = 0; voxelX < VOXEL_GRID_SIZE; voxelX++)
	{
		for (int voxelY = 0; voxelY < VOXEL_GRID_SIZE; voxelY++)
		{
			for (int voxelZ = 0; voxelZ < VOXEL_GRID_SIZE; voxelZ++)
			{
				// We hit not solid object
				if (Voxels[voxelX][voxelY][voxelZ] == 0)
					continue;

				const int maxIteration = 128;
				float traveled = 0.0;
				vec3 boxPosition = vec3(voxelX * u_Uniforms.VoxelSize * 2, voxelY * u_Uniforms.VoxelSize * 2, voxelZ * u_Uniforms.VoxelSize * 2);

				for (int i = 0; i < maxIteration; i++)
				{
					if (traveled > u_Uniforms.MaxDistance)
					{
						// We hit nothing
						break;
						// return vec4(-rayDir, 1.0);
					}
					vec3 position = rayOrig + (rayDir * traveled);
					// Check for hits
			
					float dist = distanceField(position, boxPosition);
					if (dist < hitTreshold)
					{
						vec3 normal = getNormal(position, boxPosition);
						float light = dot(-v_Input.LightDirection, normal);
						vec3 color = v_Input.LightColor * light;
						return vec4(color, 1.0);
					}
					traveled += dist;
				}
			}
		}
	}
	return vec4(-rayDir, 1.0);
}


void main() 
{
	vec4 color = rayMarch2(v_Input.RayOrigin, v_Input.Ray);

	o_Color = color;
}