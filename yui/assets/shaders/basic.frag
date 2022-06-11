#version 410 core

flat in float Frag_UseSampler;
in vec2 Frag_UV;
in vec4 Frag_Color;
layout(location = 0) out vec4 Out_Color;

uniform sampler2D sampler;

void main()
{
	if (Frag_UseSampler >= 1.0) {
		vec4 sampled = vec4(1.0, 1.0, 1.0, texture(sampler, Frag_UV).r);
		Out_Color = Frag_Color * sampled;
		//Out_Color = vec4(0.0, 0.0, 1.0, 1.0);
	}
	else {
		Out_Color = Frag_Color;
	}
}