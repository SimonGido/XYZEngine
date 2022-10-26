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

}
void EntryPoint(
{

}
void EntryPoint(
{
Vec4ToVec3(input,output);
Vec4ToVec3(input,output);
TranslationMatrix(translation,scale,transform);
SplitTransform(transform,transformRow0,transformRow1,transformRow2);

}
