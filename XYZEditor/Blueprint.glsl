struct Output
{
	vec4 TransformRow0;
	vec4 TransformRow1;
	vec4 TransformRow2;
	vec4 Color;
};
struct Input
{
	vec4 StartPosition;
	vec4 StartColor;
	vec4 StartRotation;
	vec4 StartScale;
	vec4 StartVelocity;
	vec4 EndColor;
	vec4 EndRotation;
	vec4 EndScale;
	vec4 EndVelocity;
	vec4 Position;
	float LifeTime;
	float LifeRemaining;
};
layout(std430, set = 0, binding = 0) buffer buffer_Particles
{
	Output output[];

};
layout(std430, set = 0, binding = 0) buffer buffer_ParticleProperties
{
	Input input[];

};
void Vec4ToVec3(in vec4 input,out vec3 output)
{
vec3 output = input.xyz;

}
void TranslationMatrix( vec3 translation, vec3 scale,inout mat4 transform)
{
transform = mat4(
	vec4(scale.x, 0.0, 0.0, 0.0),
	vec4(0.0, scale.y, 0.0, 0.0),
	vec4(0.0, 0.0, scale.z, 0.0),
	vec4(translation, 1.0));

}
void SplitTransform(in mat4 transform,inout vec4 transformRow0,inout vec4 transformRow1,inout vec4 transformRow2)
{
transformRow0 = vec4(transform[0][0], transform[1][0], transform[2][0], transform[3][0]);
transformRow1 = vec4(transform[0][1], transform[1][1], transform[2][1], transform[3][1]);
transformRow2 = vec4(transform[0][2], transform[1][2], transform[2][2], transform[3][2]);

}
void EntryPoint()
{
vec3 output;
Vec4ToVec3(input,output);
vec3 output;
Vec4ToVec3(input,output);
mat4 transform;
TranslationMatrix(translation,scale,transform);
vec4 transformRow0;
vec4 transformRow1;
vec4 transformRow2;
SplitTransform(transform,transformRow0,transformRow1,transformRow2);

}
layout(local_size.x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
	EntryPoint();
}