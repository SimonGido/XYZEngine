//#type vertex
#version 450


layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

struct Particle
{
    // Data that are rendered
	vec4  TransformRow0;
    vec4  TransformRow1;
    vec4  TransformRow2;
    vec4  Color;
};

layout (std430, binding = 6) buffer buffer_Particles
{
    Particle Particles[];
};

struct VertexOutput
{
	vec4 Color;
	vec3 Position;
	vec3 Normal;
	vec2 TexCoord;
	vec2 TexOffset;
	vec3 CameraPosition;
};

layout(location = 0) out VertexOutput v_Output;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_Projection;
	mat4 u_View;
	vec3 u_CameraPosition;
};

layout(push_constant) uniform Transform
{
	mat4 Transform;

} u_Renderer;


void main()
{
	int id = gl_InstanceIndex;
	mat4 instanceTransform = mat4(
		vec4(Particles[id].TransformRow0.x, Particles[id].TransformRow1.x, Particles[id].TransformRow2.x, 0.0),
		vec4(Particles[id].TransformRow0.y, Particles[id].TransformRow1.y, Particles[id].TransformRow2.y, 0.0),
		vec4(Particles[id].TransformRow0.z, Particles[id].TransformRow1.z, Particles[id].TransformRow2.z, 0.0),
		vec4(Particles[id].TransformRow0.w, Particles[id].TransformRow1.w, Particles[id].TransformRow2.w, 1.0)
	);

	vec4 instancePosition = u_Renderer.Transform * instanceTransform * vec4(a_Position, 1.0);

	v_Output.Color = Particles[id].Color;
	v_Output.Position = instancePosition.xyz;
	v_Output.Normal = a_Normal;
	v_Output.TexCoord  = a_TexCoord;
	v_Output.TexOffset = vec2(0.0, 0.0);
	v_Output.CameraPosition = u_CameraPosition;
	gl_Position = u_ViewProjection * instancePosition;
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Position;

struct VertexOutput
{
	vec4 Color;
	vec3 Position;
	vec3 Normal;
	vec2 TexCoord;
	vec2 TexOffset;
	vec3 CameraPosition;
};
layout(location = 0) in VertexOutput v_Input;

#include "Resources/Shaders/Includes/PBR.glsl"

layout(push_constant) uniform Uniform
{
	layout(offset = 64)
	vec2 Tiles;

} u_Uniforms;


layout(std140, binding = 1) uniform RendererData
{
	int  TilesCountX;
	bool ShowLightComplexity;
};

layout(std140, binding = 2) buffer buffer_PointLightsData
{
	uint NumberPointLights;
	PointLight PointLights[MAX_POINT_LIGHTS];
};

layout(std430, binding = 4) readonly buffer buffer_VisibleLightIndices
{
	int Indices[];
} visibleLightIndicesBuffer;

PBRParameters m_Params;


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

vec3 CalculatePointLights(in vec3 F0)
{
	vec3 result = vec3(0.0);
	for (int i = 0; i < NumberPointLights; i++)
	{
		uint lightIndex = GetLightBufferIndex(i);
		if (lightIndex == -1)
			break;

		PointLight light = PointLights[lightIndex];
		result += CalculatePointLight(F0, light, m_Params, v_Input.Position);
	}
	return result;
}

layout (binding = 5) uniform sampler2D u_AlbedoTexture;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);

void main()
{
	vec4 albedoColor = texture(u_AlbedoTexture, v_Input.TexCoord) * v_Input.Color;
	m_Params.Albedo = albedoColor.rgb;
	m_Params.Metalness = 0.7;
	m_Params.Roughness = 0.6;
	m_Params.Normal = normalize(v_Input.Normal);
	m_Params.View = normalize(v_Input.CameraPosition - v_Input.Position);
	m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);


	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

	// Fresnel reflectance, metals use albedo
	vec3 F0 = mix(Fdielectric, m_Params.Albedo, m_Params.Metalness);

	vec3 lightContribution = CalculatePointLights(F0) + m_Params.Albedo;
	vec3 iblContribution = IBL(F0, Lr, m_Params);


	o_Color = vec4(iblContribution * lightContribution, albedoColor.a);
	o_Position = vec4(v_Input.Position, 1.0);
}