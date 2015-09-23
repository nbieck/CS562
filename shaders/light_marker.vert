#version 440

layout(location = 0) in vec3 m_position;

struct LightSphereData
{
    mat4 model;
    vec3 color;
};

layout(std140, binding = 0) buffer data
{
    LightSphereData d[];
};

uniform mat4 ViewProj;

flat out int instance_ID;

void main()
{
    gl_Position = ViewProj * d[gl_InstanceID].model * vec4(m_position, 1);

    instance_ID = gl_InstanceID;
}
