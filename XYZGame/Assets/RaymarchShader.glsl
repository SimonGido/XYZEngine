#type vertex
#version 450

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
	vec4 u_BackgroundColor;

	//Voxel chunk info
	vec4  u_ChunkPosition;
	uint  u_MaxTraverse;
	uint  u_Width;
	uint  u_Height;
	uint  u_Depth;
	float u_VoxelSize;
	float u_Padding[3];
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
	v_Output.RayOrigin = u_CameraPosition.xyz - u_ChunkPosition.xyz;
	v_Output.LightDirection = u_LightDirection.xyz;
	v_Output.LightColor = u_LightColor.xyz;
}
#type fragment#version 450

layout(location = 0) out vec4 o_Color;



struct VertexOutput
{
	vec3 Ray;
	vec3 RayOrigin;

	vec3 LightDirection;
	vec3 LightColor;
};

struct HitResult
{
	vec3 Normal;
	uint Voxel;
};



layout(location = 0) in VertexOutput v_Input;


layout(std140, binding = 0) uniform Scene
{
	mat4 u_ViewProjection;
	mat4 u_InverseView;
	mat4 u_CameraFrustum;
	vec4 u_CameraPosition;
	vec4 u_LightDirection;
	vec4 u_LightColor;
	vec4 u_BackgroundColor;

	//Voxel chunk info
	vec4  u_ChunkPosition;
	uint  u_MaxTraverse;
	uint  u_Width;
	uint  u_Height;
	uint  u_Depth;
	float u_VoxelSize;
	float u_Padding[3];
};

layout(std430, binding = 3) buffer buffer_Voxels
{
	uint Voxels[];
};


uint Index3D(int x, int y, int z)
{
	return x + u_Width * (y + u_Depth * z);
}

uint Index3D(ivec3 index)
{
	return Index3D(index.x, index.y, index.z);
}


bool IsInsideChunk(ivec3 voxel)
{
	vec3 current_voxel = vec3(voxel.x, voxel.y, voxel.z);
	return ((current_voxel.x < float(u_Width ) && current_voxel.x > 0)
		 && (current_voxel.y < float(u_Height) && current_voxel.y > 0)
		 && (current_voxel.z < float(u_Depth)  && current_voxel.z > 0));
}


vec4 VoxelToColor(uint voxel)
{
	vec4 color;
	color.x = bitfieldExtract(voxel, 0, 8);
	color.y = bitfieldExtract(voxel, 8, 8);
	color.z = bitfieldExtract(voxel, 16, 8);
	color.w = bitfieldExtract(voxel, 24, 8);
	color /= 255.0;
	return color;
}

vec4 HitResultToColor(in HitResult hit)
{
	float light = dot(-v_Input.LightDirection, hit.Normal);
	vec4 voxelColor = VoxelToColor(hit.Voxel);
	vec3 color = voxelColor.rgb * v_Input.LightColor * light;
	return vec4(color, voxelColor.a);
}

bool RayMarch(out HitResult result, inout ivec3 currentVoxel, ivec3 step, inout vec3 tMax, inout vec3 tDelta, inout uint numTraverses)
{
	result.Voxel = 0;
	do 
	{
		if (tMax.x < tMax.y && tMax.x < tMax.z) 
		{
			result.Normal = vec3(float(-step.x), 0.0, 0.0);
			tMax.x += tDelta.x;
			currentVoxel.x += step.x;
		}
		else if (tMax.y < tMax.z) 
		{
			result.Normal = vec3(0.0, float(-step.y), 0.0);
			tMax.y += tDelta.y;
			currentVoxel.y += step.y;			
		}
		else 
		{
			result.Normal = vec3(0.0, 0.0, float(-step.z));
			tMax.z += tDelta.z;
			currentVoxel.z += step.z;		
		}
		
		// Is inside voxel area
		if (IsInsideChunk(currentVoxel))
		{
			result.Voxel = Voxels[Index3D(currentVoxel)];
			if (result.Voxel != 0)
				return true;
		}
		numTraverses += 1;
	} 
	while (result.Voxel == 0 && numTraverses < u_MaxTraverse);

	return false;
}

vec4 HitColor(vec3 rayOrig, vec3 rayDir)
{
	ivec3 current_voxel = ivec3(floor(rayOrig / u_VoxelSize));

	ivec3 step = ivec3(
		(rayDir.x > 0.0) ? 2 : -2,
		(rayDir.y > 0.0) ? 2 : -2,
		(rayDir.z > 0.0) ? 2 : -2
	);
	vec3 next_boundary = vec3(
		float((step.x > 0) ? current_voxel.x + 1 : current_voxel.x) * u_VoxelSize,
		float((step.y > 0) ? current_voxel.y + 1 : current_voxel.y) * u_VoxelSize,
		float((step.z > 0) ? current_voxel.z + 1 : current_voxel.z) * u_VoxelSize
	);

	vec3 t_max = (next_boundary - rayOrig) / rayDir; // we will move along the axis with the smallest value
	vec3 t_delta = u_VoxelSize / rayDir * vec3(step);
	uint numTraverses = 0U;

	bool first = true;
	vec4 result = u_BackgroundColor;

	HitResult hit;
	if (RayMarch(hit, current_voxel, step, t_max, t_delta, numTraverses))
	{
		result = HitResultToColor(hit);
	}
	if (result.a < 1.0)
	{
		while (RayMarch(hit, current_voxel, step, t_max, t_delta, numTraverses))
		{
			vec4 color = HitResultToColor(hit);
			result.rgb = mix(result.rgb, color.rgb, 1.0 - color.a);
			
			if (!(color.a < 1.0)) // We hit solid object, stop marching
			{
				break;
			}
		}	
	}
	return result;
}



void main() 
{
	vec4 color = HitColor(v_Input.RayOrigin, v_Input.Ray);
	o_Color = color;
}