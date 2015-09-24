#version 440

uniform sampler2D Position;
uniform sampler2D Normal;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D Shininess;

struct LightData
{
    mat4 model_mat;
    vec3 position;
    float intensity;
    vec3 color;    
    float max_distance;
};

layout(std140, binding = 0) buffer data
{
    LightData Light[];
};

uniform vec3 CamPos;

out vec3 OutColor;

flat in int instance_ID;

vec3 CalcDiffuse(vec3 L, vec3 N, vec3 diff_color)
{
    return max(dot(L, N), 0) * diff_color * Light[instance_ID].color * Light[instance_ID].intensity;
}

vec3 CalcSpecular(vec3 H, vec3 N, vec3 spec_color, float shininess)
{
    return pow(max(dot(H, N), 0), shininess) * Light[instance_ID].color * spec_color * Light[instance_ID].intensity;
}

float CalcAttenuation(float distance)
{
    float comp_factor = clamp(1.0 - (distance/Light[instance_ID].max_distance), 0, 1);

    return comp_factor * comp_factor;
}

void main()
{
    ivec2 pixel_pos = ivec2(gl_FragCoord.xy);

    vec3 Pos = texelFetch(Position, pixel_pos, 0).xyz;

    float dist = length(Light[instance_ID].position - Pos);
    if (dist > Light[instance_ID].max_distance)
        discard;

    vec3 L = normalize(Light[instance_ID].position - Pos);
    vec3 N = normalize(texelFetch(Normal, pixel_pos, 0).xyz);
    
    vec3 DiffColor = texelFetch(Diffuse, pixel_pos, 0).xyz;
    vec3 SpecColor = texelFetch(Specular, pixel_pos, 0).xyz;
    float Shine = texelFetch(Shininess, pixel_pos, 0).x;

    vec3 V = normalize(CamPos - Pos);
    vec3 H = normalize(V + L);

    float attenuation = CalcAttenuation(dist);

    OutColor = CalcDiffuse(L, N, DiffColor) * attenuation;
    OutColor += CalcSpecular(V, N, SpecColor, Shine) * attenuation;
        
}
