#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

layout(location = 5) in ivec4 a_BoneIDs;
layout(location = 6) in vec4  a_Weights;

XYZ_INSTANCED layout(location = 7) in vec4  a_TransformRow0;
XYZ_INSTANCED layout(location = 8) in vec4  a_TransformRow1;
XYZ_INSTANCED layout(location = 9) in vec4  a_TransformRow2;

const int MAX_BONES = 60;
const int MAX_ANIMATED_MESHES = 1024;

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


layout (std140, set = 2, binding = 0) readonly buffer BoneTransforms
{
	mat4 BoneTransforms[MAX_BONES * MAX_ANIMATED_MESHES];
} r_BoneTransforms;


layout(push_constant) uniform Transform
{
	mat4 Transform;
	uint BoneIndex;

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

	mat4 boneTransform = r_BoneTransforms.BoneTransforms[(u_Renderer.BoneIndex + gl_InstanceIndex) * MAX_BONES + a_BoneIDs[0]] * a_Weights[0];
	boneTransform     += r_BoneTransforms.BoneTransforms[(u_Renderer.BoneIndex + gl_InstanceIndex) * MAX_BONES + a_BoneIDs[1]] * a_Weights[1];
	boneTransform     += r_BoneTransforms.BoneTransforms[(u_Renderer.BoneIndex + gl_InstanceIndex) * MAX_BONES + a_BoneIDs[2]] * a_Weights[2];
	boneTransform     += r_BoneTransforms.BoneTransforms[(u_Renderer.BoneIndex + gl_InstanceIndex) * MAX_BONES + a_BoneIDs[3]] * a_Weights[3];



	vec4 instancePosition = transform * boneTransform * u_Renderer.Transform * vec4(a_Position, 1.0);
	
	v_Output.Position	  = instancePosition.xyz;
	v_Output.Normal		  = mat3(transform) * mat3(boneTransform) * a_Normal;
	v_Output.TexCoord	  = a_TexCoord;
	v_Output.WorldNormals = mat3(transform) * mat3(a_Tangent, a_Binormal, a_Normal);
	v_Output.Binormal	  = mat3(boneTransform) * a_Binormal;

	gl_Position = u_ViewProjection * instancePosition;
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Position;

const int MAX_POINT_LIGHTS = 1024;

struct VertexOutput
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoord;
	
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
};

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

vec3 fresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} 

vec3 IBL(vec3 F0, vec3 Lr)
{
	vec3 F = fresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
	return kd;
}


layout(location = 0) in VertexOutput v_Input;


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

	// Normals (currently from vertex)
	m_Params.Normal = normalize(v_Input.Normal);

	m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);

	// Specular reflection vector
	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

	// Fresnel reflectance, metals use albedo
	vec3 F0 = mix(Fdielectric, m_Params.Albedo, m_Params.Metalness);

	vec3 iblContribution = IBL(F0, Lr);
	o_Color = vec4(iblContribution, 1.0);
	o_Position = vec4(v_Input.Position, 1.0);
}	