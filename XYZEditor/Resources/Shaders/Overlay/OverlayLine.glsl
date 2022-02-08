#type vertex
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;



struct VertexOutput
{
	vec4 Color;
};

layout(location = 0) out VertexOutput v_Output;

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
	gl_Position = u_ViewProjection * u_Renderer.Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 430 core

layout(location = 0) out vec4 o_Color;


struct VertexOutput
{
	vec4 Color;
};
layout(location = 0) in VertexOutput v_Input;


void main()
{
	o_Color = v_Input.Color;
}
