#type vertex
#version 430


layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;

layout(location = 2) in vec4  a_IColor;
layout(location = 3) in vec2  a_IPosition;
layout(location = 4) in vec2  a_ITexCoord;
layout(location = 5) in vec2  a_ISize;
layout(location = 6) in float a_IAngle;
layout(location = 7) in float a_IAlignment;


out vec4 v_Color;
out vec2 v_TexCoord;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

uniform mat4 u_Transform;

float GetRadians(float angleInDegrees)
{
	return angleInDegrees * 3.14 / 180.0;
}
mat2 RotationZ(float angle)
{
	return mat2(cos(angle), sin(angle), -sin(angle), cos(angle));
}

void main()
{
	vec2 pos    = RotationZ(GetRadians(a_IAngle)) * a_Position.xy * a_ISize;
	gl_Position = u_ViewProjection * u_Transform * vec4(pos.x + a_IPosition.x, pos.y + a_IPosition.y, 0.0, 1.0);
	v_Color		= a_IColor;
	v_TexCoord  = a_TexCoord + a_ITexCoord;
}


#type fragment
#version 430

layout(location = 0) out vec4 o_Color;

in vec4 v_Color;
in vec2 v_TexCoord;

layout(binding = 0) uniform sampler2D u_Texture;

void main()
{
	o_Color = texture(u_Texture, v_TexCoord) * v_Color;
}

