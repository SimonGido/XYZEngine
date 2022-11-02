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
void main( ivec3 workgroups)
{

}
