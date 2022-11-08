//#type vertex
#version 450


layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

struct Particle
{
    // Data that are rendered
	vec4  TransformRow0;
    vec4  TransformRow1;
    vec4  TransformRow2;
    vec4  Color;
};

layout (std430, binding = 6) buffer buffer_Particles
{
    Particle Particles[];
};

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
	int id = gl_InstanceIndex;
	mat4 instanceTransform = mat4(
		vec4(Particles[id].TransformRow0.x, Particles[id].TransformRow1.x, Particles[id].TransformRow2.x, 0.0),
		vec4(Particles[id].TransformRow0.y, Particles[id].TransformRow1.y, Particles[id].TransformRow2.y, 0.0),
		vec4(Particles[id].TransformRow0.z, Particles[id].TransformRow1.z, Particles[id].TransformRow2.z, 0.0),
		vec4(Particles[id].TransformRow0.w, Particles[id].TransformRow1.w, Particles[id].TransformRow2.w, 1.0)
	);

	vec4 instancePosition = u_Renderer.Transform * instanceTransform * vec4(a_Position, 1.0);

	v_Output.Color = Particles[id].Color;
	v_Output.Position = instancePosition.xyz;
	v_Output.TexCoord  = a_TexCoord;
	v_Output.TexOffset = vec2(0.0, 0.0);
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