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
out float v_TilingFactor;



uniform mat4 u_ViewProjection;

void main()
{
	v_Color = a_Color;
	v_Position = a_Position.xyz;
	v_TexCoord = a_TexCoord;
	v_TextureID = a_TextureID;
	v_TilingFactor = a_TilingFactor;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);

}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Position;


in vec4 v_Color;
in vec3 v_Position;
in vec2 v_TexCoord;
in flat float v_TextureID;
in float v_TilingFactor;

uniform vec4 u_Color;
uniform sampler2D u_Texture[32];


void main()
{
	vec4 color = v_Color * u_Color;
	switch (int(v_TextureID))
	{
	case  0: color *= texture(u_Texture[0], v_TexCoord * v_TilingFactor); break;
	case  1: color *= texture(u_Texture[1], v_TexCoord * v_TilingFactor); break;
	case  2: color *= texture(u_Texture[2], v_TexCoord * v_TilingFactor); break;
	case  3: color *= texture(u_Texture[3], v_TexCoord * v_TilingFactor); break;
	case  4: color *= texture(u_Texture[4], v_TexCoord * v_TilingFactor); break;
	case  5: color *= texture(u_Texture[5], v_TexCoord * v_TilingFactor); break;
	case  6: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  7: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  8: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  9: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  10: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  11: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  12: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  13: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  14: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  15: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  16: color *= texture(u_Texture[0], v_TexCoord * v_TilingFactor); break;
	case  17: color *= texture(u_Texture[1], v_TexCoord * v_TilingFactor); break;
	case  18: color *= texture(u_Texture[2], v_TexCoord * v_TilingFactor); break;
	case  19: color *= texture(u_Texture[3], v_TexCoord * v_TilingFactor); break;
	case  20: color *= texture(u_Texture[4], v_TexCoord * v_TilingFactor); break;
	case  21: color *= texture(u_Texture[5], v_TexCoord * v_TilingFactor); break;
	case  22: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  23: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  24: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  25: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  26: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  27: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  28: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  29: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  30: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  31: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	}
	o_Color = color;
	o_Position = vec4(v_Position, 1.0);
}