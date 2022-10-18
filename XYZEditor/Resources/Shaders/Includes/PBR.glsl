
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

vec3 IBL(in vec3 F0, in vec3 Lr, in PBRParameters params)
{
	vec3 F = FresnelSchlickRoughness(F0, params.NdotV, params.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - params.Metalness);
	return kd;
}


vec3 CalculatePointLight(in vec3 F0, in PointLight light, in PBRParameters params, in vec3 position)
{
	vec3 Li = normalize(light.Position - position);
	float lightDistance = length(light.Position - position);
	vec3 Lh = normalize(Li + params.View);
	
	float attenuation = clamp(1.0 - (lightDistance * lightDistance) / (light.Radius * light.Radius), 0.0, 1.0);
	attenuation *= mix(attenuation, 1.0, light.Falloff);
	
	vec3 Lradiance = light.Radiance * light.Multiplier * attenuation;
	
	// Calculate angles between surface normal and various light vectors.
	float cosLi = max(0.0, dot(params.Normal, Li));
	float cosLh = max(0.0, dot(params.Normal, Lh));
	
	vec3 F = FresnelSchlickRoughness(F0, max(0.0, dot(Lh, params.View)), params.Roughness);
	float D = NdfGGX(cosLh, params.Roughness);
	float G = GaSchlickGGX(cosLi, params.NdotV, params.Roughness);
	
	vec3 kd = (1.0 - F) * (1.0 - params.Metalness);
	vec3 diffuseBRDF = kd * params.Albedo;
	
	// Cook-Torrance
	vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * params.NdotV);
	specularBRDF = clamp(specularBRDF, vec3(0.0f), vec3(10.0f));
	vec3 result = (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	
	return result;
}