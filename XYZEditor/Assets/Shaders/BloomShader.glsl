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

in vec2 v_TexCoords;


uniform float u_Exposure;
layout (binding = 0) uniform sampler2D u_Texture;

const vec3 c_Luminance = vec3(0.2126, 0.7152, 0.0722);

void main()
{    
    vec4 color = texture(u_Texture, v_TexCoords);      

    float brightness = dot(color.rgb, c_Luminance);
	if (brightness > 1.0)
		o_Color = vec4(color.xyz * u_Exposure, color.w);
	else
		o_Color = vec4(0.0,0.0,0.0,0.0);

}