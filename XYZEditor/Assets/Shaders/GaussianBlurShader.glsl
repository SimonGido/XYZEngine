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

uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

uniform int u_Horizontal;
uniform sampler2D u_Texture;

void main()
{             
     vec2 tex_offset = 1.0 / vec2(128,128); // gets size of single texel
     vec3 result = texture(u_Texture, v_TexCoords).rgb * weight[0];
     if (u_Horizontal == 0)
     {
         for(int i = 1; i < 5; ++i)
         {
            result += texture(u_Texture, v_TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(u_Texture, v_TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
         }
     }
     else
     {
         for(int i = 1; i < 5; ++i)
         {
             result += texture(u_Texture, v_TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
             result += texture(u_Texture, v_TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
         }
     }
     o_Color = vec4(result, 1.0);
}