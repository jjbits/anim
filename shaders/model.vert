#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 baseColorFactor;
    vec4 mrFactors;       // x=metallic, y=roughness
    vec4 emissiveFactor;
} pc;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;  // Unused, kept for layout compatibility
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

void main() {
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    fragPosition = worldPos.xyz;
    gl_Position = ubo.proj * ubo.view * worldPos;
    fragNormal = mat3(pc.model) * inNormal;
    fragUV = inUV;
}
