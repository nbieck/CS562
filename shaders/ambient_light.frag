#version 440

const float PI = 3.14159265359;

uniform sampler2D Position;
uniform sampler2D Normal;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D Shininess;

uniform sampler2D Irradiance;
uniform sampler2D Skysphere;

uniform vec3 CamPos;

out vec3 LightAccumulation;

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

vec3 BRDF_Diff(vec3 K_S)
{
    return K_S / PI;
}

vec3 BRDF(vec3 K_S, vec3 K_D, float alpha, vec3 L, vec3 V, vec3 H, vec3 N)
{
    vec3 diff = K_D / PI;
    float L_dot_H = pos_dot(L, H);
    vec3 spec = D(H, N, alpha) * F(K_S, L, H) / (4 * L_dot_H * L_dot_H);

    return diff + spec;
}

vec2 DirToUV(vec3 direction)
{
    direction = normalize(direction);
    return vec2(0.5 - atan(direction.x, direction.z) / (2 * PI), acos(-direction.y) / PI);
}

void main()
{
    ivec2 pixel_pos = ivec2(gl_FragCoord.xy);

    vec3 Pos = texelFetch(Position, pixel_pos, 0).xyz;
    
    vec3 N = normalize(texelFetch(Normal, pixel_pos, 0).xyz);

    vec3 DiffColor = texelFetch(Diffuse, pixel_pos, 0).rgb;

    LightAccumulation = BRDF_Diff(DiffColor) * textureLod(Irradiance, DirToUV(N), 0).rgb;
 
}
