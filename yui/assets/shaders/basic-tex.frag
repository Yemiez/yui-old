#version 410 core

in vec4 Frag_Color;
layout(location = 0) out vec4 Out_Color;
uniform sampler2D texture;

void main()
{
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
	Out_Color = Frag_Color;
}