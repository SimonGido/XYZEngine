//#type vertex
#version 450


layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;



layout(location = 2) in vec4  a_ITransformRow0;
layout(location = 3) in vec4  a_ITransformRow1;
layout(location = 4) in vec4  a_ITransformRow2;
layout(location = 5) in vec4  a_IColor;
layout(location = 6) in vec2  a_ITexOffset;



struct VertexOutput
{
	vec4 Color;
	vec3 Position;
	vec2 TexCoord;
	vec2 TexOffset;
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
		vec4(a_ITransformRow0.x, a_ITransformRow1.x, a_ITransformRow2.x, 0.0),
		vec4(a_ITransformRow0.y, a_ITransformRow1.y, a_ITransformRow2.y, 0.0),
		vec4(a_ITransformRow0.z, a_ITransformRow1.z, a_ITransformRow2.z, 0.0),
		vec4(a_ITransformRow0.w, a_ITransformRow1.w, a_ITransformRow2.w, 1.0)
	);

	vec4 instancePosition = transform * u_Renderer.Transform * vec4(a_Position, 1.0);

	v_Output.Color = a_IColor;
	v_Output.Position = instancePosition.xyz;
	v_Output.TexCoord  = a_TexCoord;
	v_Output.TexOffset = a_ITexOffset;
	gl_Position = u_ViewProjection * instancePosition;
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Position;

struct VertexOutput
{
	vec4 Color;
	vec3 Position;
	vec2 TexCoord;
	vec2 TexOffset;
};
layout(location = 0) in VertexOutput v_Input;

layout(push_constant) uniform Uniform
{
	layout(offset = 64)
	vec2 Tiles;

} u_Uniforms;


layout(binding = 1) uniform sampler2D u_Texture;

void main()
{
	vec2 texCoord = v_Input.TexOffset + (v_Input.TexCoord / u_Uniforms.Tiles);
	vec4 color = texture(u_Texture, texCoord) * v_Input.Color;
	if (color.a < 0.2)
		discard;

	o_Color = color;
	o_Position = vec4(v_Input.Position, 1.0);
	
}

