#version 440

in vec3 w_position;
in vec3 w_normal;
in vec2 uv;

uniform sampler2D DiffuseTex;

out vec4 out_color;

void main()
{
    out_color = texture(DiffuseTex, uv);
}
