//#type compute
#version 460

layout (binding = 0, rgba16f) uniform image2D texture_Source;
layout (binding = 1, rgba8)   uniform image2D texture_Destination;


const vec3 c_Luminance = vec3(0.2126, 0.7152, 0.0722);

uniform float u_Treshold;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

void main()
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	
	vec4 color = imageLoad(texture_Source, storePos);
	float brightness = dot(color.rgb, c_Luminance);
	if (brightness > u_Treshold)
	{
		imageStore(texture_Destination, storePos, color);
	}
	else
	{
		imageStore(texture_Destination, storePos, vec4(0.0));
	}	
}