#version 440

layout(location = 0) in vec3 m_position;

out float vtx_depth;

uniform mat4 M;
uniform mat4 VP;
uniform float near;
uniform float far;
uniform vec3 light_pos;

uniform float offset;

void main()
{
    vec4 world_pos = M * vec4(m_position, 1);
    gl_Position = VP * world_pos;

    vtx_depth = (gl_Position.w + offset - near) / (far - near);
}
