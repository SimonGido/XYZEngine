#type vertex
#version 430 core

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;


out vec2 v_TexCoords;

void main()
{
    v_TexCoords = a_TexCoord;
    gl_Position = vec4(a_Position, 1.0);
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

uniform sampler2D u_Texture;

uniform int u_NumberOfLights;

uniform vec2 u_ViewportSize;
uniform mat4 u_ViewMatrix;


void main()
{    
    vec4 color = texture(u_Texture, v_TexCoords);      
	for (int i = 0; i < u_NumberOfLights; ++i)
	{
		vec4 position = u_ViewMatrix * vec4(PointLights[i].Position.xy,0,1);
		vec2 coord = gl_FragCoord.xy * 4 / u_ViewportSize;
		vec4 viewCoord = u_ViewMatrix * vec4(coord,0,1);
		float dist = distance(coord , position.xy);
		float strength = (1.0 / dist) * PointLights[i].Intensity;
		color.rgb += PointLights[i].Color * strength; 
	}
	o_Color = color;
}