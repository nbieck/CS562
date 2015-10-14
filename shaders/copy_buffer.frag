#version 440

uniform sampler2D LightAccumulation;
uniform sampler2D Position;
uniform sampler2D Normal;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D Shininess;

uniform int BufferToShow;

out vec4 outColor;

void main()
{
    ivec2 texel_coord = ivec2(gl_FragCoord.xy);

    if (BufferToShow == 0)
    {
        vec4 c = texelFetch(LightAccumulation, texel_coord, 0);
        outColor = pow(c / (c + vec4(1)), vec4(1.0 / 2.2));
    }
    else if (BufferToShow == 1)
        outColor = abs(texelFetch(Position, texel_coord, 0) / vec4(200, 140, 115, 1));
    else if (BufferToShow == 2)
        outColor = abs(texelFetch(Normal, texel_coord, 0));
    else if (BufferToShow == 3)
        outColor = texelFetch(Diffuse, texel_coord, 0);
    else if (BufferToShow == 4)
        outColor = texelFetch(Specular, texel_coord, 0);
    else if (BufferToShow == 5)
        outColor = texelFetch(Shininess, texel_coord, 0);
    else
        outColor = vec4(1, 0.5, 0.5, 1);
}
