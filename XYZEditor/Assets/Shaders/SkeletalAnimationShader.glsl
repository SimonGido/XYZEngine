#type vertex
#version 450

layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;
layout(location = 2) in ivec4 a_BoneIDs;
layout(location = 3) in vec4  a_Weights;


out vec4 v_Position;
out vec2 v_TexCoord;

const int MAX_BONES = 60;
uniform mat4 u_Bones[MAX_BONES];
uniform mat4 u_Transform;


layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_ViewMatrix;
	vec4 u_ViewPosition;
};

void main()
{
	mat4 boneTransform = u_Bones[a_BoneIDs[0]] * a_Weights[0];
	boneTransform += u_Bones[a_BoneIDs[1]] * a_Weights[1];
	boneTransform += u_Bones[a_BoneIDs[2]] * a_Weights[2];
	boneTransform += u_Bones[a_BoneIDs[3]] * a_Weights[3];

	mat4 transform = u_Transform * boneTransform;
	vec4 position = transform * vec4(a_Position, 1.0);
	gl_Position = u_ViewProjection * position;

	v_Position = position;
	v_TexCoord = a_TexCoord;
}


#type fragment
#version 450

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_Position;

in vec4 v_Position;
in vec2 v_TexCoord;


uniform vec4 u_Color;
layout(binding = 0) uniform sampler2D u_Texture;

void main()
{
	o_Color = texture(u_Texture, v_TexCoord) * u_Color;
	o_Position = v_Position;
}