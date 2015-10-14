#version 440

const float PI = 3.14159265359;

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

float pos_dot(vec3 a, vec3 b)
{
    return max(dot(a,b),0);
}

float D(vec3 H, vec3 N, float alpha)
{
    return (alpha + 2)/(2 * PI) * pow(pos_dot(N, H), alpha);
}

vec3 F(vec3 K_S, vec3 L, vec3 H)
{
    return K_S + (1 - K_S) * pow(1 - pos_dot(L, H), 5);
}

vec3 BRDF(vec3 K_S, vec3 K_D, float alpha, vec3 L, vec3 V, vec3 H, vec3 N)
{
    vec3 diff = K_D / PI;
    float L_dot_H = pos_dot(L, H);
    vec3 spec = D(H, N, alpha) * F(K_S, L, H) / (4 * L_dot_H * L_dot_H);

    return diff + spec;
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
    vec3 V = normalize(CamPos - Pos);
    vec3 H = normalize(V + L);

    if (pos_dot(L, H) == 0)
        discard;

    vec3 N = normalize(texelFetch(Normal, pixel_pos, 0).xyz);
    
    vec3 DiffColor = texelFetch(Diffuse, pixel_pos, 0).xyz;
    vec3 SpecColor = texelFetch(Specular, pixel_pos, 0).xyz;
    float Shine = texelFetch(Shininess, pixel_pos, 0).x;

    float attenuation = CalcAttenuation(dist);

    vec3 L_i = attenuation * Light[instance_ID].color * Light[instance_ID].intensity;
    
    OutColor = L_i * pos_dot(N, L) * BRDF(SpecColor, DiffColor, Shine, L, V, H, N);
        
}
