#type vertex
#version 450


layout(location = 0) in vec3  a_Color;
layout(location = 1) in vec3  a_Position;
layout(location = 2) in vec2  a_TexCoord;


out vec3 v_Color;
out vec2 v_TexCoord;


uniform mat4 u_ViewProjectionMatrix;

void main()
{
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	gl_Position = u_ViewProjectionMatrix * vec4(a_Position, 1.0);

}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;


in vec3 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
	o_Color = texture(u_Texture, v_TexCoord) * vec4(v_Color, 1.0);
}
