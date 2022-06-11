#version 330 core

layout(location = 0) in vec2 Position;
layout(location = 1) in vec4 Color;

uniform mat4 ProjMtx;
out vec4 Frag_Color;

void main()
{
	Frag_Color = Color;
	gl_Position = ProjMtx * vec4(Position.xy, 0, 1);
}