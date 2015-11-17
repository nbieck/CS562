#version 440


uniform sampler2D PositionBuffer;
uniform sampler2D NormalBuffer;

uniform float delta = 0.001;
uniform float R = 1;
uniform float c = 0.1;
uniform int n = 20;
uniform float s = 1.0;
uniform float k = 1.0;
uniform float W;
uniform float H;


const float PI = 3.14159265359;

out float AO;

void main()
{
    ivec2 texel_loc = ivec2(gl_FragCoord.xy);
    vec2 uv = vec2(texel_loc) / vec2(W,H);

    vec4 p_val = texelFetch(PositionBuffer, texel_loc, 0);
    vec3 P = p_val.xyz;
    float d = p_val.w;

    vec3 N = texelFetch(NormalBuffer, texel_loc, 0).xyz;

    float inv_occlusion = 0;

    int rho = (30 * texel_loc.x ^ texel_loc.y) + 10 * texel_loc.x * texel_loc.y;
    //sum up contributions
    for (int i = 0; i < n; ++i)
    {
        float alpha = (i + 0.5) / n;
        float h = alpha * R / d;
        float theta = 2 * PI * alpha * (7.0 * n / 9.0) + rho;

        vec4 pos_i = texture(PositionBuffer, uv + h * vec2(cos(theta), sin(theta)));
        vec3 P_i = pos_i.xyz;
        float d_i = pos_i.w;

        vec3 omega_i = P_i - P;

        float  occlusion_term = max(0, dot(N, omega_i) - delta * d_i) * step(length(omega_i), R) / max(c * c, dot(omega_i, omega_i));

        inv_occlusion += occlusion_term;
    }
    
    inv_occlusion *= 2 * PI * c / n;

    float occlusion = pow(max(0,1 - s * inv_occlusion), k);

    AO = occlusion;
}
