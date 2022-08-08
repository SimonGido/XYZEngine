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