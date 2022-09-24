// Shadow Map shader

#type vertex
#version 450 core

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;



layout(location = 2) in vec4  a_ITransformRow0;
layout(location = 3) in vec4  a_ITransformRow1;
layout(location = 4) in vec4  a_ITransformRow2;
layout(location = 5) in vec4  a_IColor;
layout(location = 6) in vec2  a_ITexOffset;

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
	mat4 transform = mat4(
		vec4(a_ITransformRow0.x, a_ITransformRow1.x, a_ITransformRow2.x, 0.0),
		vec4(a_ITransformRow0.y, a_ITransformRow1.y, a_ITransformRow2.y, 0.0),
		vec4(a_ITransformRow0.z, a_ITransformRow1.z, a_ITransformRow2.z, 0.0),
		vec4(a_ITransformRow0.w, a_ITransformRow1.w, a_ITransformRow2.w, 1.0)
	);

	vec4 worldPosition = transform * u_Renderer.Transform * vec4(a_Position, 1.0);

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