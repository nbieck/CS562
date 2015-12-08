#version 440

layout(binding = 1) uniform sampler2D Reflection;

out vec3 Lighting;

void main()
{
    ivec2 texelCoord = ivec2(gl_FragCoord.xy);

    Lighting = texelFetch(Reflection, texelCoord, 0).rgb;
}
