#type vertex
#version 450

layout(location = 0) in vec3   a_Color;
layout(location = 1) in vec3   a_Position;
layout(location = 2) in vec2   a_TexCoord;

out vec3 v_Color;
out vec2 v_TexCoord;


uniform mat4 u_Transform;
uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);

	v_Color = a_Color;
	v_TexCoord = a_TexCoord;

}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;


in vec3 v_Color;
in vec2 v_TexCoord;

uniform int u_ColorEnabled;
uniform sampler2D u_Texture;

void main()
{
	if (bool(u_ColorEnabled))
		o_Color = vec4(v_Color, 1.0f);
	else
		o_Color = texture(u_Texture, v_TexCoord);
}
