#version 440

const float PI = 3.14159265359;

in vec3 uv;

out vec4 OutColor;

uniform sampler2D sky_tex;

void main()
{
    vec3 N = normalize(uv);
    vec2 map_uv = clamp(vec2(0.5 - atan(N.x, N.z) / (2 * PI), acos(-N.y) / PI), vec2(0), vec2(1));
    OutColor = textureLod(sky_tex, map_uv, 0);
}
