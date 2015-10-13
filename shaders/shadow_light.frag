#version 440

uniform sampler2D Position;
uniform sampler2D Normal;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D Shininess;

struct LightData
{
    vec3 position;
    float intensity;
    vec3 color;    
    float max_distance;
    float inner_cos;
    float outer_cos;
    vec3 direction;
};

uniform LightData Light;

uniform sampler2D shadow_map;
uniform mat4 shadow_mat;
uniform float shadow_near;
uniform float shadow_far;

uniform float c;

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

float SpotEffect(vec3 L)
{
    float light_cos = dot(L, Light.direction);

    if (light_cos > Light.inner_cos)
        return 1.0;
    if (light_cos < Light.outer_cos)
        return 0.0;

    return (light_cos - Light.outer_cos) / (Light.inner_cos - Light.outer_cos);
}

float Shadow(vec3 P, float dist)
{
    vec4 light_proj_P = shadow_mat * vec4(P, 1);
    float map_depth = textureProj(shadow_map, light_proj_P.xyw).r;

    float obj_depth = (light_proj_P.w - shadow_near) / (shadow_far - shadow_near);

    obj_depth = exp(-c * obj_depth);

    float shadow = obj_depth * map_depth;

    if (shadow > 1.0)
        return 1.0;

    return shadow;
}

void main()
{
    ivec2 pixel_pos = ivec2(gl_FragCoord.xy);

    vec3 Pos = texelFetch(Position, pixel_pos, 0).xyz;

    float dist = length(Light.position - Pos);
    if (dist > Light.max_distance)
        discard;

    vec3 L = normalize(Light.position - Pos);
    vec3 N = normalize(texelFetch(Normal, pixel_pos, 0).xyz);
    
    vec3 DiffColor = texelFetch(Diffuse, pixel_pos, 0).xyz;
    vec3 SpecColor = texelFetch(Specular, pixel_pos, 0).xyz;
    float Shine = texelFetch(Shininess, pixel_pos, 0).x;

    vec3 V = normalize(CamPos - Pos);
    vec3 H = normalize(V + L);

    float attenuation = CalcAttenuation(dist) * SpotEffect(L) * Shadow(Pos, dist);

    OutColor = CalcDiffuse(L, N, DiffColor) * attenuation;
    OutColor += CalcSpecular(V, N, SpecColor, Shine) * attenuation;
}

