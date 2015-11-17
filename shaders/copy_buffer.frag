#version 440

uniform sampler2D LightAccumulation;
uniform sampler2D Position;
uniform sampler2D Normal;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D Shininess;
uniform sampler2D AO_NonBlur;
uniform sampler2D AO_HorizontalBlur;
uniform sampler2D AO_Final;

uniform int BufferToShow;

uniform float exposure;
uniform float contrast;

out vec4 outColor;

void main()
{
    ivec2 texel_coord = ivec2(gl_FragCoord.xy);

    if (BufferToShow == 0)
    {
        vec4 c = texelFetch(LightAccumulation, texel_coord, 0);
        outColor = pow(exposure * c / (exposure * c + vec4(1)), vec4(contrast / 2.2));
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
    else if (BufferToShow == 6)
        outColor = vec4(vec3(texelFetch(AO_NonBlur, texel_coord, 0).r), 1);
    else if (BufferToShow == 7)
        outColor = vec4(vec3(texelFetch(AO_HorizontalBlur, texel_coord, 0).r), 1);
    else if (BufferToShow == 8)
        outColor = vec4(vec3(texelFetch(AO_Final, texel_coord, 0).r), 1);
    else
        outColor = vec4(1, 0.5, 0.5, 1);
}
