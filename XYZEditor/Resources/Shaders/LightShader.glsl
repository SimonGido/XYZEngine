#type vertex
#version 430 core

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;


layout(location = 0) out vec2 v_TexCoords;

layout(push_constant) uniform Transform
{
	mat4 Transform;
} u_Renderer;

void main()
{
	gl_Position = u_Renderer.Transform * vec4(a_Position.xy, 0.0, 1.0);
	v_TexCoords = a_TexCoord;
}


#type fragment
#version 430
#define PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 o_Color;

layout(location = 0) in vec2 v_TexCoords;


layout(push_constant) uniform LightInfo
{	
	layout(offset = 64)
	int NumberPointLights;
	int NumberSpotLights;
} u_Uniforms;


struct PointLightData
{
	vec4  Color;
	vec2  Position;
	float Radius;
	float Intensity;
};

struct SpotLightData
{
	vec4  Color;
	vec2  Position;
	float Radius;
	float Intensity;
	float InnerAngle;
	float OuterAngle;

	float Alignment[2];
};

layout(std430, binding = 1) buffer buffer_PointLights
{
	uint NumberPointLights;
	uint Offset[3];
	PointLightData PointLights[];
};

layout(std430, binding = 2) buffer buffer_SpotLights
{
	uint NumberSpotLights;
	SpotLightData SpotLights[];
};


float Determinant(vec2 a, vec2 b)
{
	return a.x * b.y - a.y * b.x;
}

vec3 CalculatePointLights(vec3 defaultColor, vec2 fragPos)
{
	vec3 litColor = vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < u_Uniforms.NumberPointLights; ++i)
	{
		float dist = distance(fragPos, PointLights[i].Position.xy);
		float radius = PointLights[i].Radius;
		if (dist <= radius)
			litColor += defaultColor.xyz * PointLights[i].Color.xyz * abs(radius - dist) * PointLights[i].Intensity;
	}
	return litColor;
}

vec3 CalculateSpotLights(vec3 defaultColor, vec2 fragPos)
{
	vec3 litColor = vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < u_Uniforms.NumberSpotLights; ++i)
	{
		float dist = distance(fragPos, SpotLights[i].Position.xy);
		float radius = SpotLights[i].Radius;
		if (dist <= radius)
		{
			vec2 toVertexDir = normalize(fragPos - SpotLights[i].Position.xy);
			vec2 toCenter = vec2(0.0, 1.0);
			float dotProduct = dot(toVertexDir, toCenter);
			float deter = Determinant(toVertexDir, toCenter);
			float angle = atan(deter, dotProduct) * (180.0 / PI);
			if (angle > SpotLights[i].InnerAngle && angle < SpotLights[i].OuterAngle)
				litColor += defaultColor.xyz * SpotLights[i].Color.xyz * abs(radius - dist) * SpotLights[i].Intensity;
		}
	}
	return litColor;
}




layout(binding = 0) uniform sampler2D u_Texture[2];

void main()
{
	vec4 color = texture(u_Texture[0], v_TexCoords);
	vec3 fragPos = texture(u_Texture[1], v_TexCoords).xyz;

	vec3 litColor = CalculatePointLights(color.xyz, fragPos.xy);
	litColor += CalculateSpotLights(color.xyz, fragPos.xy);

	o_Color = vec4(litColor, color.a);
}