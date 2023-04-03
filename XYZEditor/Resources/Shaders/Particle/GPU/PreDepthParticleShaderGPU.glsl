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

layout(location = 0) out VertexOutput v_Output;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_Projection;
	mat4 u_View;
	vec3 u_CameraPosition;
};

layout(push_constant) uniform Transform
{
	mat4 Transform;

} u_Renderer;

layout(location = 0) out float v_LinearDepth;

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

	v_LinearDepth = -(u_View * instancePosition).z;
	gl_Position = u_ViewProjection * instancePosition;
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_LinearDepth;

layout(location = 0) in float v_LinearDepth;

void main()
{
	// TODO: Check for alpha in texture
	o_LinearDepth = vec4(v_LinearDepth, 0.0, 0.0, 1.0);
}