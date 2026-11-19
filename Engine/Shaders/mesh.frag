#version 450

layout(set = 0, binding = 0) uniform Camera{ mat4 view; mat4 projection; vec4 lightPos; } camera;

layout(push_constant) uniform Push { mat4 model; vec4 params; vec4 baseColor; } push;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragWorldPos;
layout(location = 0) out vec4 outColor;

void main()
{
    if (push.params.w > 0.5)
    {
        outColor = vec4(push.params.xyz, 1.0);
        return;
    }

    vec3 n = normalize(fragNormal);

    vec3 toLight = camera.lightPos.xyz - fragWorldPos;
    float dist = length(toLight);
    vec3 l = toLight / dist;

    vec3 camPos = inverse(camera.view)[3].xyz;
    vec3 v = normalize(camPos - fragWorldPos);
    vec3 h = normalize(l + v);

    float ndl = max(dot(n, l), 0.0);
    float attenuation = 1.0 / (1.0 + 0.35 * dist * dist);
    float intensity = 3.0;

    vec3 albedo = push.baseColor.xyz;
    vec3 ambient = albedo * mix(vec3(0.06, 0.06, 0.08), vec3(0.18, 0.18, 0.22), n.y * 0.5 + 0.5);
    vec3 diffuse = albedo * ndl * attenuation * intensity;
    float spec = pow(max(dot(n, h), 0.0), 48.0) * step(0.0, ndl) * attenuation * intensity;

    vec3 color = ambient + diffuse + vec3(spec) * 0.5;
    color = color / (color + vec3(1.0));
    outColor = vec4(color, 1.0);
}