#type vertex
#version 430 core

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;


out vec2 v_TexCoords;
out vec3 v_Normal;

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

void main()
{    
    vec4 color = texture(u_Texture, v_TexCoords);    
	float aspect = u_ViewportSize.x / u_ViewportSize.y;
	vec2 worldFragPos = vec2(
		(gl_FragCoord.x - (u_ViewportSize.x / 2.0)) / (u_ViewportSize.x / 2.0),
		(gl_FragCoord.y - (u_ViewportSize.y / 2.0)) / (u_ViewportSize.y / 2.0)
	);
																	
	for (int i = 0; i < u_NumberOfLights; ++i)
	{		
		float dist = distance(worldFragPos, PointLights[i].Position.xy);
		float strength = (1.0 / dist) * PointLights[i].Intensity;
		color.rgb += PointLights[i].Color * clamp(strength,0,1); 
	}
	o_Color = color;
}