#version 450

precision highp float;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = color;
}