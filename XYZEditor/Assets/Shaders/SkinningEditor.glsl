#type vertex
#version 450


layout(location = 0) in vec3   a_Color;
layout(location = 1) in vec3   a_Position;
layout(location = 2) in vec2   a_TexCoord;
layout(location = 3) in ivec4  a_BoneIDs;
layout(location = 4) in vec4   a_Weights;

out vec3 v_Color;
out vec2 v_TexCoord;

//const int MAX_BONES = 60;
//uniform int u_BonesEnabled;
//uniform mat4 u_Bones[MAX_BONES];
uniform mat4 u_Transform;
uniform mat4 u_ViewProjectionMatrix;

void main()
{
	//mat4 boneTransform = u_Bones[a_BoneIDs[0]] * a_Weights[0];
	//boneTransform += u_Bones[a_BoneIDs[1]] * a_Weights[1];
	//boneTransform += u_Bones[a_BoneIDs[2]] * a_Weights[2];
	//boneTransform += u_Bones[a_BoneIDs[3]] * a_Weights[3];

	//if (bool(u_BonesEnabled))
	//	gl_Position = u_ViewProjectionMatrix * u_Transform * boneTransform * vec4(a_Position, 1.0);
	//else
	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);

	v_Color = a_Color;
	v_TexCoord = a_TexCoord;

}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;


in vec3 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main()
{
	o_Color = texture(u_Texture, v_TexCoord) * vec4(v_Color, 1.0);
}
