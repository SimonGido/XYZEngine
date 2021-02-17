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


uniform sampler2D u_Texture[2];

void main()
{    
    vec4 geometryColor = texture(u_Texture[0], v_TexCoords);
    vec4 bloomColor = texture(u_Texture[1], v_TexCoords);
    geometryColor += bloomColor;
    geometryColor.a = clamp(geometryColor.a, 0.0, 1.0);
    o_Color = geometryColor;
}