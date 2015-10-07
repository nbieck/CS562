#version 440

layout(location = 0) in vec3 m_position;

uniform mat4 model_mat;

uniform mat4 ViewProj;

void main()
{
    gl_Position = ViewProj * model_mat * vec4(m_position, 1);
}

