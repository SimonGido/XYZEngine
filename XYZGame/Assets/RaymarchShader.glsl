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

	//Voxel chunk info
	vec4  ChunkPosition;
	uint  MaxTraverse;
	uint  Width;
	uint  Height;
	uint  Depth;
	float VoxelSize;
	float Padding[3];
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



struct VertexOutput
{
	vec3 Ray;
	vec3 RayOrigin;

	vec3 LightDirection;
	vec3 LightColor;
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

	//Voxel chunk info
	vec4  ChunkPosition;
	uint  MaxTraverse;
	uint  Width;
	uint  Height;
	uint  Depth;
	float VoxelSize;
	float Padding[3];
};

layout(std430, binding = 3) buffer buffer_Voxels
{
	uint Voxels[];
};


uint Index3D(int x, int y, int z)
{
	return x + Width * (y + Depth * z);
}

uint Index3D(ivec3 index)
{
	return Index3D(index.x, index.y, index.z);
}


bool IsInsideChunk(ivec3 voxel)
{
	vec3 current_voxel = vec3(voxel.x, voxel.y, voxel.z);
	return ((current_voxel.x < float(Width ) && current_voxel.x > 0)
		 && (current_voxel.y < float(Height) && current_voxel.y > 0)
		 && (current_voxel.z < float(Depth)  && current_voxel.z > 0));
}

uint extractInt(uint orig, uint from, uint to)
{
	uint mask = ((1 << (to - from + 1)) - 1) << from;
	return (orig & mask) >> from;
}

vec4 VoxelToColor(uint voxel)
{
	vec4 color;
	color.x = float(extractInt(voxel, 0,  8))  / 255.0;
	color.y = float(extractInt(voxel, 8,  16)) / 255.0;
	color.z = float(extractInt(voxel, 16, 24)) / 255.0;
	color.w = float(extractInt(voxel, 24, 32)) / 255.0;

	return color;
}

vec4 RayMarch(vec3 rayOrig, vec3 rayDir)
{
	vec4 defaultColor = vec4(0.3, 0.2, 0.7, 1.0);

	vec3 origin = rayOrig + ChunkPosition.xyz;
	vec3 direction = rayDir;
	ivec3 current_voxel = ivec3(floor(origin / VoxelSize));

	ivec3 step = ivec3(
		(direction.x > 0.0) ? 1 : -1,
		(direction.y > 0.0) ? 1 : -1,
		(direction.z > 0.0) ? 1 : -1
	);
	vec3 next_boundary = vec3(
		float((step.x > 0) ? current_voxel.x + 1 : current_voxel.x) * VoxelSize,
		float((step.y > 0) ? current_voxel.y + 1 : current_voxel.y) * VoxelSize,
		float((step.z > 0) ? current_voxel.z + 1 : current_voxel.z) * VoxelSize
	);

	vec3 t_max = (next_boundary - origin) / direction; // we will move along the axis with the smallest value
	vec3 t_delta = VoxelSize / direction * vec3(step);
	vec3 normal;
	uint voxel = 0;
	uint i = 0;
	do 
	{
		if (t_max.x < t_max.y && t_max.x < t_max.z) 
		{
			normal = vec3(float(-step.x), 0.0, 0.0);
			t_max.x += t_delta.x;
			current_voxel.x += step.x;
		}
		else if (t_max.y < t_max.z) 
		{
			normal = vec3(0.0, float(-step.y), 0.0);
			t_max.y += t_delta.y;
			current_voxel.y += step.y;			
		}
		else 
		{
			normal = vec3(0.0, 0.0, float(-step.z));
			t_max.z += t_delta.z;
			current_voxel.z += step.z;		
		}
		
		// Is inside voxel area
		if (IsInsideChunk(current_voxel))
		{
			voxel = Voxels[Index3D(current_voxel)];
			if (voxel != 0)
			{
				float light = dot(-v_Input.LightDirection, normal);
				vec4 voxelColor = VoxelToColor(voxel);
				vec3 color = voxelColor.xyz * v_Input.LightColor * light;
				return vec4(color, voxelColor.a);
			}
		}
		i += 1;
	} 
	while (voxel == 0 && i < MaxTraverse);
	
	return defaultColor;
}



void main() 
{
	vec4 color = RayMarch(v_Input.RayOrigin, v_Input.Ray);
	o_Color = color;
}