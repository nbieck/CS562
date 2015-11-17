#version 440

in vec3 w_position;
in vec3 w_normal;
in vec2 uv;
in float d;

uniform sampler2D DiffuseTex;
uniform sampler2D SpecularTex;

uniform float SpecularCoefficient;

uniform bool HasDiffTex;
uniform bool HasSpecTex;
uniform vec3 K_D;
uniform vec3 K_S;

layout(location = 0) out vec3 LightAccumulation;
layout(location = 1) out vec4 Position;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 Diffuse;
layout(location = 4) out vec3 Specular;
layout(location = 5) out float Shininess;

void main()
{
    LightAccumulation = vec3(0);
    Position = vec4(w_position, d);
    Normal = normalize(w_normal);
    Diffuse = pow(texture(DiffuseTex, uv).rgb, vec3(2.2));
    if (!HasDiffTex)
        Diffuse = K_D;
    Specular = pow(texture(SpecularTex, uv).rgb, vec3(2.2));
    if (!HasSpecTex)
        Specular = K_S;
    Shininess = SpecularCoefficient;
}
