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
	vec4  Position;
	vec3  Color;
	float Intensity;
};

layout(std430, binding = 0) buffer
buffer_PointLights
{
	PointLightData PointLights[];
};

in vec2 v_TexCoords;

uniform sampler2D u_Texture[2];
uniform int u_NumberOfLights;

const float c_Constant = 1.0;
const float c_Linear = 0.09;
const float c_Quadratic = 0.032;

void main()
{
	vec4 color = texture(u_Texture[0], v_TexCoords);
	vec3 fragPos = texture(u_Texture[1], v_TexCoords).xyz;

	for (int i = 0; i < u_NumberOfLights; ++i)
	{
		vec3 ambient = color.xyz;
		float distance = length(PointLights[i].Position.xyz - fragPos);
		float attenuation = 1.0 / (c_Constant + c_Linear * distance + c_Quadratic * (distance * distance));
		vec3 lightDir = normalize(PointLights[i].Position.xyz - fragPos);
		vec3 diffuse = color.xyz;
	
		ambient *= attenuation;
		diffuse *= attenuation;
	
		color.xyz += (ambient + diffuse) * PointLights[i].Color * PointLights[i].Intensity;
	}
	o_Color = color;
}