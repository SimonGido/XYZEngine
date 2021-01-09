#type vertex
layout(std140, binding = 0) uniform XYZ_OrthoCamera
{
	mat4 ViewProjectionMatrix;
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	vec2 ViewPos;
};


#type fragment
layout(std140, binding = 0) uniform XYZ_Test
{
	mat4 Test;
	mat4 Test2;
	mat4 Test3;
	vec2 Test4;
};
