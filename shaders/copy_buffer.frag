#version 440

uniform sampler2D LightAccumulation;
uniform sampler2D Position
uniform sampler2D Normal;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D Shininess;

uniform int BufferToShow;

out vec4 outColor;

void main()
{
    if (BufferToShow == 0)
        outColor = texelFetch(LightAccumulation, gl_FragCoord.xy, 0);
    else if (BufferToShow == 1)
        outColor = texelFetch(Position, gl_FragCoord.xy, 0);
    else if (BufferToShow == 2)
        outColor = texelFetch(Normal, gl_FragCoord.xy, 0);
    else if (BufferToShow == 3)
        outColor = texelFetch(Diffuse, gl_FragCoord.xy, 0);
    else if (BufferToShow == 4)
        outColor = texelFetch(Specular, gl_FragCoord.xy, 0);
    else if (BufferToShow == 5)
        outColor = texelFetch(Shininess, gl_FragCoord.xy, 0);
    else
        outColor = vec4(1, 0.5, 0.5, 1);
}
