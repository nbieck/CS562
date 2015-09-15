#version 440

in vec3 w_position;
in vec3 w_normal;
in vec2 uv;

uniform sampler2D DiffuseTex;

layout(location = 0) out vec3 LightAccumulation;
layout(location = 1) out vec3 Position;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 Diffuse;
layout(location = 4) out vec3 Specular;
layout(location = 5) out float Shininess;

void main()
{
    LightAccumulation = vec3(0);
    Position = w_position;
    Normal = normalize(w_normal);
    Diffuse = texture(DiffuseTex, uv).rgb;
    Specular = vec3(0);
    Shininess = 128;
}
