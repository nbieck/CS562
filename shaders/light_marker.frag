#version 440

uniform vec3 color;

layout(location = 0) out vec3 LightAccumulation;
layout(location = 1) out vec3 Position;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 Diffuse;
layout(location = 4) out vec3 Specular;
layout(location = 5) out float Shininess;

void main()
{
    LightAccumulation = color;
    Position = vec3(0);
    Normal = vec3(0);
    Diffuse = vec3(0);
    Specular = vec3(0);
    Shininess = 0;
}

