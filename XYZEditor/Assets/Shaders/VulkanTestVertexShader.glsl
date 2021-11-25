#type vertex
#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 texCoord;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_ViewMatrix;
	vec4 u_ViewPosition;
};

layout(std140, set = 1, binding = 1) uniform Test
{
	mat4 u_ViewProjectionTest;
	mat4 u_ViewMatrixTest;
	vec4 u_ViewPositionTest;
};

layout(push_constant) uniform Transform
{
	mat4 Transform;
} u_Renderer;

void main() 
{
    gl_Position = u_ViewProjectionTest * u_Renderer.Transform * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
	texCoord = inTexCoord;
}

#type fragment
#version 450
layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;
layout(binding = 2) uniform sampler2D u_Texture;

layout(push_constant) uniform Uniforms
{
	layout(offset = 64) vec4 Color;

} u_Uniforms;

void main() 
{
    outColor = texture(u_Texture, inTexCoord) * u_Uniforms.Color;
}