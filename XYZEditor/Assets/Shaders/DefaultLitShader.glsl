#type vertex
#version 450


layout(location = 0) in vec4  a_Color;
layout(location = 1) in vec3  a_Position;
layout(location = 2) in vec2  a_TexCoord;
layout(location = 3) in float a_TextureID;
layout(location = 4) in float a_TilingFactor;


out vec4 v_Color;
out vec3 v_Position;
out vec2 v_TexCoord;
out flat float v_TextureID;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_ViewMatrix;
	vec4 u_ViewPosition;
};

void main()
{
	v_Color = a_Color;
	v_Position = a_Position;
	v_TexCoord = a_TexCoord * a_TilingFactor;
	v_TextureID = a_TextureID;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);

}

#type fragment
#version 450
#define PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Position;


in vec4 v_Color;
in vec3 v_Position;
in vec2 v_TexCoord;
in flat float v_TextureID;

uniform vec4 u_Color;

layout(binding = 0) uniform sampler2D u_Texture[32];

void main()
{
	vec4 color = v_Color * u_Color * texture(u_Texture[int(v_TextureID)], v_TexCoord);
	o_Color = color;
	o_Position = vec4(v_Position, 1.0);
}