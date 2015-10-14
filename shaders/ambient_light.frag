#version 440

uniform sampler2D Diffuse;

uniform vec3 AmbientLight;

out vec3 LightAccumulation;

void main()
{
    LightAccumulation = pow(AmbientLight, vec3(2.2)) * texelFetch(Diffuse, ivec2(gl_FragCoord.xy), 0).rgb;
}
