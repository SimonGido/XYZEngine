#type vertex
#version 430 core

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;


out vec2 v_TexCoords;


void main()
{
    gl_Position =  vec4(a_Position, 1.0);
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

void main()
{    
    vec4 color = texture(u_Texture[0], v_TexCoords);    			
	vec3 fragPos = texture(u_Texture[1], v_TexCoords).xyz;

	for (int i = 0; i < u_NumberOfLights; ++i)
	{		
		float strength = (1.0 / distance(PointLights[i].Position.xy, fragPos.xy)) * PointLights[i].Intensity;
		strength = clamp(strength, 0.0, 1.0);
		color.rgb += PointLights[i].Color * strength;
	}
	o_Color = color;
}