#version 440

in vec2 uv;

out vec4 out_color;

uniform sampler2D shadow_map;

void main()
{
    out_color = vec4(vec3(texture(shadow_map, uv).r), 1);
}
