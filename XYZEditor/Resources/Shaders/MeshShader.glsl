#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

layout(location = 5) in vec4  a_TransformRow0;
layout(location = 6) in vec4  a_TransformRow1;
layout(location = 7) in vec4  a_TransformRow2;



struct VertexOutput
{
	vec3 Position;
	vec2 TexCoord;
};

layout(location = 0) out VertexOutput v_Output;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_ViewMatrix;
	vec4 u_ViewPosition;
};

layout(push_constant) uniform Transform
{
	mat4 Transform;
} u_Renderer;

void main()
{
	mat4 transform = mat4(
		vec4(a_TransformRow0.x, a_TransformRow1.x, a_TransformRow2.x, 0.0),
		vec4(a_TransformRow0.y, a_TransformRow1.y, a_TransformRow2.y, 0.0),
		vec4(a_TransformRow0.z, a_TransformRow1.z, a_TransformRow2.z, 0.0),
		vec4(a_TransformRow0.w, a_TransformRow1.w, a_TransformRow2.w, 1.0)
	);


	vec4 instancePosition = transform * u_Renderer.Transform * vec4(a_Position, 1.0);
	v_Output.Position = instancePosition.xyz;
	v_Output.TexCoord = a_TexCoord;

	gl_Position = u_ViewProjection * instancePosition;
}

#type fragment
#version 450


layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Position;


struct VertexOutput
{
	vec3 Position;
	vec2 TexCoord;
};

layout(location = 0) in VertexOutput v_Input;

layout(binding = 1) uniform sampler2D u_Texture;

void main()
{
	o_Color = texture(u_Texture, v_Input.TexCoord);
	o_Position = vec4(v_Input.Position, 1.0);
}