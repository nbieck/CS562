#version 440

layout(location = 0, binding = 1) uniform sampler2D depth_buff;

layout(location = 0) out vec2 min_max;

void main()
{
    min_max = vec2(texelFetch(depth_buff, ivec2(gl_FragCoord.xy), 0).r);
}

