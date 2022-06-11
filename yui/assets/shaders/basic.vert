#version 330 core

layout(location = 0) in vec2 Position;
layout(location = 1) in float UseSampler;
layout(location = 2) in vec2 UV;
layout(location = 3) in vec4 Color;

uniform mat4 ProjMtx;
flat out float Frag_UseSampler;
out vec2 Frag_UV;
out vec4 Frag_Color;

void main()
{
	Frag_Color = Color;
	Frag_UV = UV;
	Frag_UseSampler = UseSampler;
	gl_Position = ProjMtx * vec4(Position.xy, 0.0, 1.0);
}