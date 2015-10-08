#version 440

layout (location = 0) in vec3 m_pos;
layout (location = 1) in vec2 m_uv;

uniform mat4 MVP;

out vec2 uv;

void main()
{
    gl_Position = MVP * vec4(m_pos, 1);

    uv = m_uv;
}
