#version 440
			
layout(location = 0) in vec3 m_position;
layout(location = 1) in vec3 m_normal;

out vec3 c_normal;
out vec4 c_position;

uniform mat4 MVP;
uniform mat4 NormalMat;
uniform mat4 ModelView;

void main()
{
    gl_Position = MVP * vec4(m_position, 1);
    c_normal = (NormalMat * vec4(m_normal, 0)).xyz;
    c_position = ModelView * vec4(m_position, 1);
}