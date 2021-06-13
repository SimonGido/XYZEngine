#type vertex
#version 430 core

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;


out vec2 v_TexCoords;


void main()
{
	gl_Position = vec4(a_Position, 1.0);
	v_TexCoords = a_TexCoord;
}


#type fragment
#version 430

layout(location = 0) out vec4 o_Color;

struct PointLightData
{
	vec4  Color;
	vec2  Position;
	float Radius;
	float Intensity;
};

layout(std430, binding = 1) buffer
buffer_PointLights
{
	PointLightData PointLights[];
};

in vec2 v_TexCoords;

layout(binding = 0) uniform sampler2D u_Texture[2];

uniform int u_NumberOfLights;


void main()
{
	vec4 color = texture(u_Texture[0], v_TexCoords);
	vec3 fragPos = texture(u_Texture[1], v_TexCoords).xyz;

	vec3 result = vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < u_NumberOfLights; ++i)
	{
		float dist = distance(fragPos.xy, PointLights[i].Position.xy);
		float radius = PointLights[i].Radius;
		if (dist <= radius)
			result += color.xyz * PointLights[i].Color.xyz * abs(radius - dist) * PointLights[i].Intensity;
	}
	o_Color = vec4(result, 1.0);
}