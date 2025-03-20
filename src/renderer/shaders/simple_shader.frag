#version 450

layout (location = 0) in vec4 fragColor;   // Receive color from vertex shader
layout (location = 0) out vec4 outColor;

void main() {
    outColor = fragColor;  // Use the incoming color instead of hardcoded
}