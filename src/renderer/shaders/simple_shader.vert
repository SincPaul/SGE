#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;     // Add this line

layout (location = 0) out vec4 fragColor;  // Add this line

layout (binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;  // Pass the color to fragment shader
}