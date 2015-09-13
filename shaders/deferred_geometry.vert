#version 440

layout(location = 0) in vec3 m_position;
layout(location = 1) in vec3 m_normal;
layout(location = 2) in vec2 m_uv;

out vec3 w_position;
out vec3 w_normal;
out vec2 uv;

uniform mat4 MVP;
uniform mat4 NormalMat;
uniform mat4 Model;

void main()
{
    gl_Position = MVP * vec4(m_position, 1);
    w_normal = (NormalMat * vec4(m_normal, 0)).xyz;
    w_position = (Model * vec4(m_position, 1)).xyz;
    uv = m_uv;
}
