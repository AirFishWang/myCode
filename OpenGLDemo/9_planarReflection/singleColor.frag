#version 330 core

out vec4 color;
in vec2 TextCoord;
uniform sampler2D text;

void main()
{
	color = vec4(0.2, 0.9, 0.0, 0.4);
}