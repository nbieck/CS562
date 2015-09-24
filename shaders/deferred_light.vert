#version 440

layout(location = 0) in vec3 m_position;

struct LightData
{
    mat4 model_mat;
    vec3 position;
    float intensity;
    vec3 color;    
    float max_distance;
};

layout(std140, binding = 0) buffer data
{
    LightData Light[];
};

uniform mat4 ViewProj;

flat out int instance_ID;

void main()
{
    gl_Position = ViewProj * Light[gl_InstanceID].model_mat * vec4(m_position, 1);

    instance_ID = gl_InstanceID;
}

