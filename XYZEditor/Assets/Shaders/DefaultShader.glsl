#type vertex
#version 430


layout(location = 0) in vec4  a_Color;
layout(location = 1) in vec3  a_Position;
layout(location = 2) in vec2  a_TexCoord;
layout(location = 3) in float a_TextureID;
layout(location = 4) in float a_TilingFactor;


out vec4 v_Color;
out vec3 v_Position;
out vec2 v_TexCoord;
out float v_TextureID;
out float v_TilingFactor;


uniform mat4 u_ViewProjectionMatrix;

void main()
{
	v_Color = a_Color;
	v_Position = a_Position.xyz;
	v_TexCoord = a_TexCoord;
	v_TextureID = a_TextureID;
	v_TilingFactor = a_TilingFactor;
	gl_Position = u_ViewProjectionMatrix * vec4(a_Position, 1.0);

}

#type fragment
#version 430

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Position;


in vec4 v_Color;
in vec3 v_Position;
in vec2 v_TexCoord;
in float v_TextureID;
in float v_TilingFactor;

uniform vec4 u_Color;
uniform sampler2D u_Texture[32];


void main()
{
	o_Color = texture(u_Texture[int(v_TextureID)], v_TexCoord * v_TilingFactor) * v_Color * u_Color;
	o_Position = vec4(v_Position, 1.0);
}
