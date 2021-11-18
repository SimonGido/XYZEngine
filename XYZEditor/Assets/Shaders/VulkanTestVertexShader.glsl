#type vertex
#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;


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


void main() 
{
    gl_Position = u_ViewProjectionTest * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}

#type fragment
#version 450
layout(location = 0) in vec3 inColor;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(inColor, 1.0);
}