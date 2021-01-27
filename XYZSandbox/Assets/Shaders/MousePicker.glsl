#type vertex
#version 430 core

layout(location = 0) in float a_ObjectID;

out flat float v_ObjectID;

void main()
{
	v_ObjectID = a_ObjectID;
}


#type fragment
#version 430

layout(location = 0) out int o_IDBuffer;

in flat float a_ObjectID;


void main()
{
	o_IDBuffer = int(a_ObjectID);
}