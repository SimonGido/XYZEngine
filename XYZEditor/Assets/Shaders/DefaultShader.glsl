#type vertex
#version 450 


layout(location = 0) in vec4  a_Color;
layout(location = 1) in vec3  a_Position;
layout(location = 2) in vec2  a_TexCoord;
layout(location = 3) in float a_TextureID;
layout(location = 4) in float a_TilingFactor;



struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) out VertexOutput v_Output;
layout (location = 2) out flat float   v_TextureID;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

layout(push_constant) uniform Transform
{
	mat4 Transform;
} u_Renderer;

void main()
{
	v_Output.Color = a_Color;
	v_Output.TexCoord = a_TexCoord * a_TilingFactor;
	v_TextureID = a_TextureID;
	gl_Position = u_ViewProjection * u_Renderer.Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) in VertexOutput v_Input;
layout (location = 2) in flat float   v_TextureID;

layout(binding = 1) uniform sampler2D u_Texture[32];

void main()
{
	vec4 color = v_Input.Color * texture(u_Texture[int(v_TextureID)], v_Input.TexCoord);
	o_Color = color;
}