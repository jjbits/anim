#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() {
    // Simple directional light
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(fragNormal);

    float diffuse = max(dot(normal, lightDir), 0.0);
    float ambient = 0.2;
    float lighting = ambient + diffuse * 0.8;

    // Base color (gray for now, texture later)
    vec3 baseColor = vec3(0.8);

    outColor = vec4(baseColor * lighting, 1.0);
}
