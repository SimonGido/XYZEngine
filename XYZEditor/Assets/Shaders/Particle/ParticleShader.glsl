#type vertex
#version 430


layout(location = 0) in vec3  a_Position;
layout(location = 1) in vec2  a_TexCoord;
layout(location = 2) in vec4  a_IColor;
layout(location = 3) in vec4  a_IPosition;
layout(location = 4) in vec2  a_ITexCoordOffset;
layout(location = 5) in vec2  a_ISize;
layout(location = 6) in float a_IAngle;
layout(location = 7) in float a_IAlignment;
layout(location = 8) in float a_IAlignment2;
layout(location = 9) in float a_IAlignment3;


out vec4 v_Color;
out vec2 v_TexCoord;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;

float GetRadians(float angleInDegrees)
{
	return angleInDegrees * 3.14 / 180.0;
}
mat2 RotationZ(float angle)
{
	return mat2(
		cos(angle),
		sin(angle),
		-sin(angle),
		cos(angle)
	);
}

void main()
{
	vec2 pos = RotationZ(GetRadians(a_IAngle)) * a_Position.xy;
	pos *= a_ISize;

	gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(pos.x + a_IPosition.x,pos.y + a_IPosition.y, 0.0, 1.0);
	v_Color = a_IColor;
	v_TexCoord = a_TexCoord + a_ITexCoordOffset;
}


#type fragment
#version 430

in vec4 v_Color;
in vec2 v_TexCoord;

out vec4 FragColor;

uniform sampler2D u_Texture;

void main()
{
	gl_FragColor = texture(u_Texture, v_TexCoord) * v_Color;
}

