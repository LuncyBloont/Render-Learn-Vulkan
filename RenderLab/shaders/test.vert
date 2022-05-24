#version 450

precision highp float;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec4 fposition;
layout(location = 1) out vec4 fcolor;

void main() {
	fposition = vec4(position, 1.0);
	gl_Position = fposition;
	fcolor = vec4(color, 1.0);
}