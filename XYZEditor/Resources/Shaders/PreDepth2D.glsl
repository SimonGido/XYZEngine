// Shadow Map shader

#type vertex
#version 450 core

layout(location = 0) in vec4  a_Color;
layout(location = 1) in vec3  a_Position;
layout(location = 2) in vec2  a_TexCoord;
layout(location = 3) in float a_TextureID;
layout(location = 4) in float a_TilingFactor;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_ViewMatrix;
	vec4 u_ViewPosition;
};

layout(push_constant) uniform Transform
{
	mat4 Transform;
	uint BoneIndex;

} u_Renderer;

layout(location = 0) out float v_LinearDepth;

void main()
{
	vec4 worldPosition = u_Renderer.Transform * vec4(a_Position, 1.0);

	v_LinearDepth = -(u_ViewMatrix * worldPosition).z;
	gl_Position = u_ViewProjection * worldPosition;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_LinearDepth;

layout(location = 0) in float v_LinearDepth;

void main()
{
	// TODO: Check for alpha in texture
	o_LinearDepth = vec4(v_LinearDepth, 0.0, 0.0, 1.0);
}