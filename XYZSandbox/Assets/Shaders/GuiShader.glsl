#type vertex
#version 450


layout(location = 0) in vec4  a_Color;
layout(location = 1) in vec3  a_Position;
layout(location = 2) in vec2  a_TexCoord;
layout(location = 3) in float a_TextureID;
layout(location = 4) in float a_TilingFactor;


out vec4 v_Color;
out vec2 v_TexCoord;
out flat float v_TextureID;
out float v_TilingFactor;

uniform mat4 u_ViewProjectionMatrix;

void main()
{
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	v_TextureID = a_TextureID;
	v_TilingFactor = a_TilingFactor;
	gl_Position = u_ViewProjectionMatrix * vec4(a_Position.xy, 0.0, 1.0);
}

#type fragment
#version 450

#define PI 3.141592

// size of a square in pixel


in vec4 v_Color;
in vec2 v_TexCoord;
in flat float v_TextureID;
in float v_TilingFactor;


uniform sampler2D u_Texture[32];
out vec4 FragColor;

void main()
{
	vec2 tiledTexCoord = v_TexCoord * v_TilingFactor;
	FragColor = vec4(texture(u_Texture[int(v_TextureID)], tiledTexCoord)) * v_Color;
}

