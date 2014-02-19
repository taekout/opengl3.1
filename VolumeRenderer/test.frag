#version 130

precision highp float;

in vec3 fragColor;
out vec4 out_Color;

void main()
{
	//out_Color = vec4(1.0, 0.0, 0.0, 1.0);
	out_Color = vec4(fragColor, 1.0);
}