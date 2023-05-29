//#type compute
#version 460
#extension GL_EXT_shader_8bit_storage : enable

#include "Resources/Shaders/Includes/Random.glsl"
#include "Resources/Shaders/Includes/PBR.glsl"


#define TILE_SIZE 16

const float FLT_MAX = 3.402823466e+38;
const float EPSILON = 0.01;

// TODO: get theese from camera
const float FarClip = 1000.0;
const float NearClip = 0.1;

layout (std140, binding = 16) uniform Scene
{
	// Camera info
	mat4 u_InverseProjection;
	mat4 u_InverseView;	
	vec4 u_CameraPosition;
	vec4 u_ViewportSize;

	// Light info
	DirectionalLight u_DirectionalLight;
};

layout(push_constant) uniform SSAO
{
	uint SampleCount;
    float IndirectAmount;
    float NoiseAmount;
    bool Noise;

} u_Uniforms;


layout(binding = 0, rgba16f) uniform image2D u_Image;
layout(binding = 1, r32f) uniform image2D u_DepthImage;
layout(binding = 2, rgba16f) uniform image2D o_SSGIImage;

float fmod(float x, float y)
{
    return x - y * trunc(x/y);
}

float distToDepth(float dist)
{
    return (dist - NearClip) / (FarClip - NearClip);
}

vec2 mod_dither3(vec2 u)
{
    float noiseX = fmod(u.x + u.y + fmod(208. + u.x * 3.58, 13. + mod(u.y * 22.9, 9.)), 7.) * .143;
    float noiseY = fmod(u.y + u.x + fmod(203. + u.y * 3.18, 12. + mod(u.x * 27.4, 8.)), 6.) * .139;
    return vec2(noiseX, noiseY) * 2.0 - 1.0;
}

vec2 dither(vec2 coord, float seed, vec2 size)
{
    float noiseX = ((fract(1.0 - (coord.x + seed * 1.0) * (size.x / 2.0)) * 0.25) + (fract((coord.y + seed * 2.0) * (size.y / 2.0)) * 0.75)) * 2.0 - 1.0;
    float noiseY = ((fract(1.0 - (coord.x + seed * 3.0) * (size.x / 2.0)) * 0.75) + (fract((coord.y + seed * 4.0) * (size.y / 2.0)) * 0.25)) * 2.0 - 1.0;
    return vec2(noiseX, noiseY);
}

vec3 getViewPos(vec2 uv)
{
    vec2 coord = uv * u_ViewportSize.xy;
    float dist = imageLoad(u_DepthImage, ivec2(coord)).r;
    float depth = distToDepth(dist);

    //Turn the current pixel from ndc to world coordinates
    vec3 pixel_pos_ndc = vec3(uv * 2.0 - 1.0, depth * 2.0 - 1.0);
    vec4 pixel_pos_clip = u_InverseProjection * vec4(pixel_pos_ndc, 1.0);
    vec3 pixel_pos_cam = pixel_pos_clip.xyz / pixel_pos_clip.w;
    return pixel_pos_cam;
}

vec3 getViewNormal(vec2 uv)
{
    float pW = 1.0 / u_ViewportSize.x;
    float pH = 1.0 / u_ViewportSize.y;

    vec3 p1 = getViewPos(uv + vec2(pW, 0.0)).xyz;
    vec3 p2 = getViewPos(uv + vec2(0.0, pH)).xyz;
    vec3 p3 = getViewPos(uv + vec2(-pW, 0.0)).xyz;
    vec3 p4 = getViewPos(uv + vec2(0.0, -pH)).xyz;

    vec3 vP = getViewPos(uv);

    vec3 dx = vP - p1;
    vec3 dy = p2 - vP;
    vec3 dx2 = p3 - vP;
    vec3 dy2 = vP - p4;

    if (length(dx2) < length(dx) && uv.x - pW >= 0.0 || uv.x + pW > 1.0)
    {
        dx = dx2;
    }

    if (length(dy2) < length(dy) && uv.y - pH >= 0.0 || uv.y + pH > 1.0)
    {
        dy = dy2;
    }

    return normalize(-cross(dx, dy).xyz);
}

float lenSq(vec3 v)
{
    return pow(v.x, 2.0) + pow(v.y, 2.0) + pow(v.z, 2.0);
}


vec3 lightSample(vec2 uv, vec2 lightUV, vec3 normal, vec3 position, float n, vec2 texsize)
{
    vec2 random = vec2(1.0, 1.0);
    bool noise = u_Uniforms.Noise;
    float noiseAmount = u_Uniforms.NoiseAmount;
    
    if (noise)
    {
        random = (mod_dither3((uv * texsize) + vec2(n * 82.294, n * 127.721))) * 0.01 * noiseAmount;
    }
    else
    {
        random = dither(uv, 1.0, texsize) * 0.1 * noiseAmount;
    }
    vec2 coord = uv * u_ViewportSize.xy;
    
    lightUV *= vec2(0.7, 0.7);

    vec2 lightCoord = (lightUV + random) * u_ViewportSize.xy;
    //light absolute data 
    vec3 lightcolor = imageLoad(u_Image, ivec2(lightCoord)).rgb;

    vec3 lightnormal = getViewNormal(fract(lightUV) + random).rgb;
    vec3 lightposition = getViewPos(fract(lightUV) + random).xyz;

                //light variable data
    vec3 lightpath = lightposition - position;
    vec3 lightdir = normalize(lightpath);

                //falloff calculations
    float cosemit = clamp(dot(lightdir, -lightnormal), 0.0, 1.0); //emit only in one direction
    float coscatch = clamp(dot(lightdir, normal) * 0.5 + 0.5, 0.0, 1.0); //recieve light from one direction
    float distfall = pow(lenSq(lightpath), 0.1) + 1.0; //fall off with distance
    
    return (lightcolor * cosemit * coscatch / distfall) * (length(lightposition) / 20.0);
}

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main() 
{
    vec4 direct = imageLoad(u_Image, ivec2(gl_GlobalInvocationID.xy));
    vec2 uv = gl_GlobalInvocationID.xy / u_ViewportSize.xy;
    vec3 color = normalize(direct).rgb;
    vec3 indirect = vec3(0.0, 0.0, 0.0);
    float PI = 3.14159;
    vec2 texSize = u_ViewportSize.xy;
    
    vec3 position = getViewPos(uv);
    vec3 normal = getViewNormal(uv);
   
    int samplesCount = int(u_Uniforms.SampleCount);
    float indirectAmount = u_Uniforms.IndirectAmount;
    
    float dlong = PI * (3.0 - sqrt(5.0));
    float dz = 1.0 / float(samplesCount);
    float l = 0.0;
    float z = 1.0 - dz / 2.0;

    for (int i = 0; i < samplesCount; i++)
    {
        float r = sqrt(1.0 - z);
    
        float xpoint = (cos(l) * r) * 0.5 + 0.5;
        float ypoint = (sin(l) * r) * 0.5 + 0.5;
    
        
        z = z - dz;
        l = l + dlong;
    
        indirect += lightSample(uv, vec2(xpoint, ypoint), normal, position, float(i), texSize);
    }
    vec3 indirectColor = (indirect / float(samplesCount)) * indirectAmount;
        
    direct.rgb += indirectColor.rgb;
    
    float dist = imageLoad(u_DepthImage, ivec2(gl_GlobalInvocationID.xy)).r;
    float depth = distToDepth(dist);

    imageStore(o_SSGIImage, ivec2(gl_GlobalInvocationID.xy), vec4(direct.rgb, 1.0));

    imageStore(o_SSGIImage, ivec2(gl_GlobalInvocationID.xy), vec4(depth, depth, depth, 1.0));

}
