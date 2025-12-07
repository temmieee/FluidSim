#version 460 core
out vec4 FragColor;
layout(rgba32f, binding = 1) uniform image2D progressive;
uniform sampler2D screen;
uniform sampler2D progressiveTex;
in vec2 UVs;
void main()
{
	vec4 pixel = texture(progressiveTex, UVs);
	//imageStore(progressive, UVs, pixel);
	FragColor = texture(screen, UVs);
};