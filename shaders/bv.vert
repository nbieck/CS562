#version 440
			
layout(location = 0) in vec3 m_position;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(m_position, 1);
}