#version 440

uniform sampler2D Position;
uniform sampler2D Normal;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D Shininess;

struct LightData
{
    vec3 position;
    vec3 color;    
    float intensity;
    float max_distance;
};

uniform LightData Light;

uniform vec3 CamPos;

out vec3 OutColor;


vec3 CalcDiffuse(vec3 L, vec3 N, vec3 diff_color)
{
    return max(dot(L, N), 0) * diff_color * Light.color * Light.intensity;
}

vec3 CalcSpecular(vec3 H, vec3 N, vec3 spec_color, float shininess)
{
    return pow(max(dot(H, N), 0), shininess) * Light.color * spec_color * Light.intensity;
}

float CalcAttenuation(float distance)
{
    float comp_factor = clamp(1.0 - (distance/Light.max_distance), 0, 1);

    return comp_factor * comp_factor;
}

void main()
{
    ivec2 pixel_pos = ivec2(gl_FragCoord.xy);

    vec3 Pos = texelFetch(Position, pixel_pos, 0).xyz;

    vec3 L = normalize(Light.position - Pos);
    vec3 N = normalize(texelFetch(Normal, pixel_pos, 0).xyz);
    
    vec3 DiffColor = texelFetch(Diffuse, pixel_pos, 0).xyz;
    vec3 SpecColor = texelFetch(Specular, pixel_pos, 0).xyz;
    float Shine = texelFetch(Shininess, pixel_pos, 0).x;

    vec3 V = normalize(CamPos - Pos);
    vec3 H = normalize(V + L);

    float attenuation = CalcAttenuation(length(Light.position - Pos));

    OutColor = CalcDiffuse(L, N, DiffColor) * attenuation;
    OutColor += CalcSpecular(V, N, SpecColor, Shine) * attenuation;
        
}
