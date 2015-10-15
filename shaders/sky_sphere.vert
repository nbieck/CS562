#version 440

layout(location = 0) in vec3 m_position;

out vec3 uv;

uniform mat4 MVP;

void main()
{
    vec4 pos = MVP * vec4(m_position, 1);
    gl_Position = pos.xyww;
    uv = m_position;
}
