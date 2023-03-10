#version 440

const float PI = 3.14159265359;

in vec3 uv;

out vec4 OutColor;

uniform sampler2D sky_tex;

void main()
{
    vec3 N = normalize(uv);
    vec2 map_uv = vec2(atan(N.z, N.x) / (2 * PI) + 0.5, acos(-N.y) / PI);
    OutColor = textureLod(sky_tex, map_uv, 0);
}
