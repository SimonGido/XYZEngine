//#type vertex
#version 450


layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;

layout(location = 2) in vec4  a_TransformRow0;
layout(location = 3) in vec4  a_TransformRow1;
layout(location = 4) in vec4  a_TransformRow2;

layout(location = 5) in vec4  a_IColor;
layout(location = 6) in vec3  a_IPosition;
layout(location = 7) in vec3  a_ISize;
layout(location = 8) in vec4  a_IAxis;
layout(location = 9) in vec2  a_ITexOffset;


struct VertexOutput
{
	vec4 Color;
	vec3 Position;
	vec2 TexCoord;
};

layout(location = 0) out VertexOutput v_Output;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_ViewMatrix;
	vec4 u_ViewPosition;
};

layout(push_constant) uniform Transform
{
	mat4 Transform;

} u_Renderer;



float GetRadians(float angleInDegrees)
{
	return angleInDegrees * 3.14 / 180.0;
}

vec4 QuatFromAxisAngle(vec3 axis, float angle)
{
	vec4 qr;
	float half_angle = (angle * 0.5) * 3.14159 / 180.0;
	qr.x = axis.x * sin(half_angle);
	qr.y = axis.y * sin(half_angle);
	qr.z = axis.z * sin(half_angle);
	qr.w = cos(half_angle);
	return qr;
}

vec3 RotateVertexPosition(vec3 position, vec3 axis, float angle)
{
	vec4 q = QuatFromAxisAngle(axis, angle);
	vec3 v = position.xyz;
	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

vec3 RotateVertex(vec3 position, vec4 q)
{
	vec3 v = position.xyz;
	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

void main()
{
	mat4 transform = mat4(
		vec4(a_TransformRow0.x, a_TransformRow1.x, a_TransformRow2.x, 0.0),
		vec4(a_TransformRow0.y, a_TransformRow1.y, a_TransformRow2.y, 0.0),
		vec4(a_TransformRow0.z, a_TransformRow1.z, a_TransformRow2.z, 0.0),
		vec4(a_TransformRow0.w, a_TransformRow1.w, a_TransformRow2.w, 1.0)
	);

	vec3 worldPos = RotateVertex(a_Position * a_ISize, a_IAxis) + a_IPosition;

	vec4 instancePosition = transform * vec4(worldPos, 1.0);

	

	gl_Position = u_ViewProjection * u_Renderer.Transform * instancePosition;
	v_Output.Color = a_IColor;
	// v_Output.TexCoord  = a_ITexOffset + (a_TexCoord / u_Uniforms.Tiles);
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

struct VertexOutput
{
	vec4 Color;
	vec3 Position;
	vec2 TexCoord;
};
layout(location = 0) in VertexOutput v_Input;

layout(push_constant) uniform Uniform
{
	layout(offset = 64)
	vec2 Tiles;

} u_Uniforms;


layout(binding = 1) uniform sampler2D u_Texture;

void main()
{
	o_Color = texture(u_Texture, v_Input.TexCoord) * v_Input.Color;
}

