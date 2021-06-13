#type vertex
#version 450


layout(location = 0) in vec4  a_Color;
layout(location = 1) in vec3  a_Position;
layout(location = 2) in vec2  a_TexCoord;
layout(location = 3) in float a_TextureID;
layout(location = 4) in float a_TilingFactor;


out vec4 v_Color;
out vec3 v_Position;
out vec2 v_TexCoord;
out flat float v_TextureID;
out float v_TilingFactor;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec4 u_ViewPosition;
};

void main()
{
	v_Color = a_Color;
	v_Position = a_Position;
	v_TexCoord = a_TexCoord;
	v_TextureID = a_TextureID;
	v_TilingFactor = a_TilingFactor;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);

}

#type fragment
#version 450
#define PI 3.1415926535897932384626433832795

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Position;


in vec4 v_Color;
in vec3 v_Position;
in vec2 v_TexCoord;
in flat float v_TextureID;
in float v_TilingFactor;

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
	PointLightData PointLights[];
};

layout(std430, binding = 2) buffer buffer_SpotLights
{
	SpotLightData SpotLights[];
};


float Determinant(vec2 a, vec2 b)
{
	return a.x * b.y - a.y * b.x;
}

vec3 CalculatePointLights(vec3 defaultColor)
{
	vec3 litColor = vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < PointLights.length(); ++i)
	{
		float dist = distance(v_Position.xy, PointLights[i].Position.xy);
		float radius = PointLights[i].Radius;
		if (dist <= radius)
			litColor += defaultColor.xyz * PointLights[i].Color.xyz * abs(radius - dist) * PointLights[i].Intensity;
	}
	return litColor;
}

vec3 CalculateSpotLights(vec3 defaultColor)
{
	vec3 litColor = vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < SpotLights.length(); ++i)
	{
		float dist = distance(v_Position.xy, SpotLights[i].Position.xy);
		float radius = SpotLights[i].Radius;
		if (dist <= radius)
		{
			vec2 toVertexDir = normalize(v_Position.xy - SpotLights[i].Position.xy);
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



uniform vec4 u_Color;

layout(binding = 0) uniform sampler2D u_Texture[32];

void main()
{
	vec4 color = v_Color * u_Color;
	switch (int(v_TextureID))
	{
	case  0: color *= texture(u_Texture[0], v_TexCoord * v_TilingFactor); break;
	case  1: color *= texture(u_Texture[1], v_TexCoord * v_TilingFactor); break;
	case  2: color *= texture(u_Texture[2], v_TexCoord * v_TilingFactor); break;
	case  3: color *= texture(u_Texture[3], v_TexCoord * v_TilingFactor); break;
	case  4: color *= texture(u_Texture[4], v_TexCoord * v_TilingFactor); break;
	case  5: color *= texture(u_Texture[5], v_TexCoord * v_TilingFactor); break;
	case  6: color *= texture(u_Texture[6], v_TexCoord * v_TilingFactor); break;
	case  7: color *= texture(u_Texture[7], v_TexCoord * v_TilingFactor); break;
	case  8: color *= texture(u_Texture[8], v_TexCoord * v_TilingFactor); break;
	case  9: color *= texture(u_Texture[9], v_TexCoord * v_TilingFactor); break;
	case  10: color *= texture(u_Texture[10], v_TexCoord * v_TilingFactor); break;
	case  11: color *= texture(u_Texture[11], v_TexCoord * v_TilingFactor); break;
	case  12: color *= texture(u_Texture[12], v_TexCoord * v_TilingFactor); break;
	case  13: color *= texture(u_Texture[13], v_TexCoord * v_TilingFactor); break;
	case  14: color *= texture(u_Texture[14], v_TexCoord * v_TilingFactor); break;
	case  15: color *= texture(u_Texture[15], v_TexCoord * v_TilingFactor); break;
	case  16: color *= texture(u_Texture[16], v_TexCoord * v_TilingFactor); break;
	case  17: color *= texture(u_Texture[17], v_TexCoord * v_TilingFactor); break;
	case  18: color *= texture(u_Texture[18], v_TexCoord * v_TilingFactor); break;
	case  19: color *= texture(u_Texture[19], v_TexCoord * v_TilingFactor); break;
	case  20: color *= texture(u_Texture[20], v_TexCoord * v_TilingFactor); break;
	case  21: color *= texture(u_Texture[21], v_TexCoord * v_TilingFactor); break;
	case  22: color *= texture(u_Texture[22], v_TexCoord * v_TilingFactor); break;
	case  23: color *= texture(u_Texture[23], v_TexCoord * v_TilingFactor); break;
	case  24: color *= texture(u_Texture[24], v_TexCoord * v_TilingFactor); break;
	case  25: color *= texture(u_Texture[25], v_TexCoord * v_TilingFactor); break;
	case  26: color *= texture(u_Texture[26], v_TexCoord * v_TilingFactor); break;
	case  27: color *= texture(u_Texture[27], v_TexCoord * v_TilingFactor); break;
	case  28: color *= texture(u_Texture[28], v_TexCoord * v_TilingFactor); break;
	case  29: color *= texture(u_Texture[29], v_TexCoord * v_TilingFactor); break;
	case  30: color *= texture(u_Texture[30], v_TexCoord * v_TilingFactor); break;
	case  31: color *= texture(u_Texture[31], v_TexCoord * v_TilingFactor); break;
	}
	

	vec3 litColor = CalculatePointLights(color.xyz);
	litColor += CalculateSpotLights(color.xyz);
	
	o_Color = vec4(litColor, color.a);
	o_Position = vec4(v_Position, 1.0);
}