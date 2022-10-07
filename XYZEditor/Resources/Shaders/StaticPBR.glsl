#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

XYZ_INSTANCED layout(location = 5) in vec4  a_TransformRow0;
XYZ_INSTANCED layout(location = 6) in vec4  a_TransformRow1;
XYZ_INSTANCED layout(location = 7) in vec4  a_TransformRow2;


struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoord;
	
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
};

layout(location = 0) out VertexOutput v_Output;


layout(push_constant) uniform Transform
{
	mat4 Transform;

} u_Renderer;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_ViewMatrix;
	vec4 u_ViewPosition;
};


void main()
{
	mat4 transform = mat4(
		vec4(a_TransformRow0.x, a_TransformRow1.x, a_TransformRow2.x, 0.0),
		vec4(a_TransformRow0.y, a_TransformRow1.y, a_TransformRow2.y, 0.0),
		vec4(a_TransformRow0.z, a_TransformRow1.z, a_TransformRow2.z, 0.0),
		vec4(a_TransformRow0.w, a_TransformRow1.w, a_TransformRow2.w, 1.0)
	);


	vec4 instancePosition = transform *  u_Renderer.Transform * vec4(a_Position, 1.0);
	
	v_Output.Position	  = instancePosition.xyz;
	v_Output.Normal		  = mat3(transform) * a_Normal;
	v_Output.TexCoord	  = a_TexCoord;
	v_Output.WorldNormals = mat3(transform) * mat3(a_Tangent, a_Binormal, a_Normal);
	v_Output.Binormal	  = a_Binormal;

	gl_Position = u_ViewProjection * instancePosition;
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Position;


struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoord;
	
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
};

layout(location = 0) in VertexOutput v_Input;


const float PI = 3.141592;
const float Epsilon = 0.00001;
const int MAX_POINT_LIGHTS = 1024;



struct PointLight 
{
	vec3  Position;
	float Multiplier;
	vec3  Radiance;
	float MinRadius;
	float Radius;
	float Falloff;
	float LightSize;
	bool  CastsShadows;
};

struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;

	vec3 Normal;
	vec3 View;
	float NdotV;
};

layout(std140, binding = 1) uniform RendererData
{
	int  TilesCountX;
	bool ShowLightComplexity;
};

layout(std140, binding = 2) uniform PointLightsData
{
	uint NumberPointLights;
	PointLight PointLights[MAX_POINT_LIGHTS];
};

layout(std430, binding = 14) readonly buffer buffer_VisibleLightIndices
{
	int Indices[];
} visibleLightIndicesBuffer;

PBRParameters m_Params;

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float NdfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float GaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float GaSchlickGGX(float cosLi, float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return GaSchlickG1(cosLi, k) * GaSchlickG1(NdotV, k);
}

vec3 FresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} 

vec3 IBL(vec3 F0, vec3 Lr)
{
	vec3 F = FresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
	return kd;
}

int GetLightBufferIndex(int i)
{
	ivec2 tileID = ivec2(gl_FragCoord) / ivec2(16, 16);
	uint index = tileID.y * TilesCountX + tileID.x;

	uint offset = index * 1024;
	return visibleLightIndicesBuffer.Indices[offset + i];
}

int GetPointLightCount()
{
	int result = 0;
	for (int i = 0; i < NumberPointLights; i++)
	{
		uint lightIndex = GetLightBufferIndex(i);
		if (lightIndex == -1)
			break;

		result++;
	}
	return result;
}

vec3 GetGradient(float value)
{
	vec3 zero = vec3(0.0, 0.0, 0.0);
	vec3 white = vec3(0.0, 0.1, 0.9);
	vec3 red = vec3(0.2, 0.9, 0.4);
	vec3 blue = vec3(0.8, 0.8, 0.3);
	vec3 green = vec3(0.9, 0.2, 0.3);

	float step0 = 0.0f;
	float step1 = 2.0f;
	float step2 = 4.0f;
	float step3 = 8.0f;
	float step4 = 16.0f;

	vec3 color = mix(zero, white, smoothstep(step0, step1, value));
	color = mix(color, white, smoothstep(step1, step2, value));
	color = mix(color, red, smoothstep(step1, step2, value));
	color = mix(color, blue, smoothstep(step2, step3, value));
	color = mix(color, green, smoothstep(step3, step4, value));

	return color;
}

vec3 CalculatePointLights(in vec3 F0)
{
	vec3 result = vec3(0.0);
	for (int i = 0; i < NumberPointLights; i++)
	{
		uint lightIndex = GetLightBufferIndex(i);
		if (lightIndex == -1)
			break;

		PointLight light = PointLights[lightIndex];
		vec3 Li = normalize(light.Position - v_Input.Position);
		float lightDistance = length(light.Position - v_Input.Position);
		vec3 Lh = normalize(Li + m_Params.View);

		float attenuation = clamp(1.0 - (lightDistance * lightDistance) / (light.Radius * light.Radius), 0.0, 1.0);
		attenuation *= mix(attenuation, 1.0, light.Falloff);

		vec3 Lradiance = light.Radiance * light.Multiplier * attenuation;

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(m_Params.Normal, Li));
		float cosLh = max(0.0, dot(m_Params.Normal, Lh));

		vec3 F = FresnelSchlickRoughness(F0, max(0.0, dot(Lh, m_Params.View)), m_Params.Roughness);
		float D = NdfGGX(cosLh, m_Params.Roughness);
		float G = GaSchlickGGX(cosLi, m_Params.NdotV, m_Params.Roughness);

		vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
		vec3 diffuseBRDF = kd * m_Params.Albedo;

		// Cook-Torrance
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * m_Params.NdotV);
		specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
		result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	}
	return result;
}




// PBR texture inputs
layout (binding = 3) uniform sampler2D u_AlbedoTexture;
layout (binding = 4) uniform sampler2D u_NormalTexture;
layout (binding = 5) uniform sampler2D u_MetalnessTexture;
layout (binding = 6) uniform sampler2D u_RoughnessTexture;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);


void main()
{
	// Standard PBR inputs
	m_Params.Albedo    = texture(u_AlbedoTexture,	 v_Input.TexCoord).rgb; 
	m_Params.Metalness = texture(u_MetalnessTexture, v_Input.TexCoord).r;
	m_Params.Roughness = texture(u_RoughnessTexture, v_Input.TexCoord).r;
    m_Params.Roughness = max(m_Params.Roughness, 0.05); // Minimum roughness of 0.05 to keep specular highlight
	float alpha = texture(u_AlbedoTexture,	 v_Input.TexCoord).a;

	// Normals (currently from vertex)
	m_Params.Normal = normalize(v_Input.Normal);

	m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);

	// Specular reflection vector
	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

	// Fresnel reflectance, metals use albedo
	vec3 F0 = mix(Fdielectric, m_Params.Albedo, m_Params.Metalness);

	vec3 lightContribution = CalculatePointLights(F0) + m_Params.Albedo;
	vec3 iblContribution = IBL(F0, Lr);


	o_Color = vec4(iblContribution * lightContribution, alpha);
	o_Position = vec4(v_Input.Position, 1.0);

	if (ShowLightComplexity)
	{
		int pointLightCount = GetPointLightCount();
		float value = float(pointLightCount);
		o_Color.rgb = (o_Color.rgb * 0.2) + GetGradient(value);
	}
}	