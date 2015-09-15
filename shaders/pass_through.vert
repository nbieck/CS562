#version 440

layout(location = 0) in vec3 m_position;

void main()
{
    gl_Position = vec4(m_position, 1);
}
