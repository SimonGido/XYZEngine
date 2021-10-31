//#type vertex
#version 450


layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;

layout(location = 2) in vec4  a_IColor;
layout(location = 3) in vec3  a_IPosition;
layout(location = 4) in vec3  a_ISize;
layout(location = 5) in vec4  a_IAxis;
layout(location = 6) in vec2  a_ITexOffset;

out vec4 v_Color;
out vec2 v_TexCoord;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_ViewMatrix;
	vec4 u_ViewPosition;
};

uniform mat4 u_Transform;
uniform vec2 u_Tiles;

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
	vec3 worldPos = RotateVertex(a_Position * a_ISize, a_IAxis);
	worldPos += a_IPosition;

	gl_Position = u_ViewProjection * u_Transform * vec4(worldPos, 1.0);
	v_Color		= a_IColor;
	v_TexCoord  = a_ITexOffset + (a_TexCoord / u_Tiles);
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

in vec4 v_Color;
in vec2 v_TexCoord;

layout(binding = 0) uniform sampler2D u_Texture;

void main()
{
	o_Color = texture(u_Texture, v_TexCoord) * v_Color;
}

