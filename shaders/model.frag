#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

layout(push_constant) uniform PushConstants {
    mat4 model;
    vec4 baseColorFactor;
    vec4 mrFactors;       // x=metallic, y=roughness
    vec4 emissiveFactor;
} pc;

layout(binding = 1) uniform sampler2D baseColorTex;
layout(binding = 2) uniform sampler2D normalTex;
layout(binding = 3) uniform sampler2D metallicRoughnessTex;
layout(binding = 4) uniform sampler2D occlusionTex;
layout(binding = 5) uniform sampler2D emissiveTex;

const float PI = 3.14159265359;

// Compute TBN matrix from derivatives (when tangents not in vertex data)
mat3 computeTBN(vec3 N, vec3 pos, vec2 uv) {
    vec3 dp1 = dFdx(pos);
    vec3 dp2 = dFdy(pos);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invmax, B * invmax, N);
}

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// Geometry function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// Smith's method for geometry
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel (Schlick approximation)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // Sample textures and multiply by material factors
    vec4 baseColor = texture(baseColorTex, fragUV) * pc.baseColorFactor;
    vec3 albedo = pow(baseColor.rgb, vec3(2.2)); // sRGB to linear

    // glTF standard: G=roughness, B=metallic, multiplied by factors
    vec4 mrSample = texture(metallicRoughnessTex, fragUV);
    float metallic = mrSample.b * pc.mrFactors.x;
    float roughness = mrSample.g * pc.mrFactors.y;

    float ao = texture(occlusionTex, fragUV).r;
    vec3 emissive = texture(emissiveTex, fragUV).rgb * pc.emissiveFactor.rgb;

    // Sample normal map and transform to world space
    vec3 geomNormal = normalize(fragNormal);
    vec3 normalMap = texture(normalTex, fragUV).rgb * 2.0 - 1.0;  // [0,1] -> [-1,1]
    mat3 TBN = computeTBN(geomNormal, fragPosition, fragUV);
    vec3 N = normalize(TBN * normalMap);

    vec3 V = normalize(ubo.camPos - fragPosition);

    // F0 for dielectrics is 0.04, for metals use albedo
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Single directional light
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 lightColor = vec3(1.0, 1.0, 1.0) * 3.0;

    vec3 L = lightDir;
    vec3 H = normalize(V + L);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // Energy conservation
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic; // Metals have no diffuse

    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * lightColor * NdotL;

    // Ambient (simple IBL approximation)
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // Add emissive
    color += emissive;

    // HDR tone mapping (Reinhard)
    color = color / (color + vec3(1.0));

    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    outColor = vec4(color, baseColor.a);
}
