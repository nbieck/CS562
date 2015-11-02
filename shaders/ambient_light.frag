#version 440

const float PI = 3.14159265359;
const float epsilon = 0.0001;

uniform sampler2D Position;
uniform sampler2D Normal;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D Shininess;

uniform sampler2D Irradiance;
uniform sampler2D Skysphere;

uniform vec3 CamPos;

uniform int NumSamples;

//subroutine float Distribution(vec3 H, vec3 N, float alpha);
//subroutine vec2 Skew(vec2 uv, float alpha);
//subroutine float Geometry(vec3 L, vec3 V, vec3 H, vec3 N);
//subroutine float GeometrySmithSubfunc(vec3 I, vec3 N);

//layout (location = 0) subroutine uniform Distribution D;
//layout (location = 1) subroutine uniform Skew SkewFunc;
//layout (location = 2) subroutine uniform Geometry G;
//layout (location = 3) subroutine uniform GeometrySmithSubfunc GPrime;

layout(binding = 0, std430) buffer RandomNumbers
{
    vec2 rand_vals[];
};

out vec3 LightAccumulation;

float pos_dot(vec3 a, vec3 b)
{
    return max(dot(a,b),0);
}

vec3 F(vec3 K_S, vec3 L, vec3 H)
{
    return K_S + (1 - K_S) * pow(1 - pos_dot(L, H), 5);
}

/*subroutine(Distribution)*/ float PhongDist(vec3 H, vec3 N, float alpha)
{
    return (alpha + 2)/(2 * PI) * pow(pos_dot(N, H), alpha);
}

/*subroutine(Skew)*/ vec2 PhongSkew(vec2 uv, float alpha)
{
    return vec2(uv.x, acos(pow(uv.y, 1.0 / (alpha + 1.0))) / PI);
}

/*subroutine(Geometry)*/ float GImplicit(vec3 L, vec3 V, vec3 H, vec3 N)
{
    float L_dot_H = pos_dot(L,H);
    if (L_dot_H < epsilon)
        return 0.0;

    return pos_dot(L,N) * pos_dot(V,N) / (L_dot_H * L_dot_H);
}

/*subroutine(GeometrySmithSubfunc)*/ float Dummy(vec3 I, vec3 N)
{
    return 1.0;
}

vec3 BRDF_Diff(vec3 K_S)
{
    return K_S / PI;
}

vec3 BRDF_Spec_Monte_Carlo(vec3 L_i, vec3 K_S, vec3 L, vec3 V, vec3 H, vec3 N)
{
    float N_dot_L = pos_dot(L, N);
    float N_dot_V = pos_dot(V, V);
    if (N_dot_L < epsilon || N_dot_V < epsilon)
        return vec3(0);

    return (F(K_S, L, H) * GImplicit(L,V,H,N) /  (4 * N_dot_L * N_dot_V)) * L_i * pos_dot(L, N);
}


vec2 DirToUV(vec3 direction)
{
    direction = normalize(direction);
    return vec2(atan(direction.z, direction.x) / (2 * PI) + 0.5, acos(-direction.y) / PI);
}

vec3 DirFromUV(vec2 uv)
{
    return vec3(cos(2.0 * PI * (uv.x - 0.5)) * sin(PI * uv.y), cos(PI * uv.y), sin(2.0 * PI * (uv.x - 0.5)) * sin(PI * uv.y));
}

void main()
{
    ivec2 pixel_pos = ivec2(gl_FragCoord.xy);

    vec3 Pos = texelFetch(Position, pixel_pos, 0).xyz;
    
    vec3 N = normalize(texelFetch(Normal, pixel_pos, 0).xyz);

    vec3 DiffColor = texelFetch(Diffuse, pixel_pos, 0).rgb;

    LightAccumulation = BRDF_Diff(DiffColor) * textureLod(Irradiance, DirToUV(N), 0).rgb;

    vec3 V = normalize(CamPos - Pos);
    vec3 R = reflect(-V, N); 
    vec3 SpecColor = texelFetch(Specular, pixel_pos, 0).rgb;
    float alpha = texelFetch(Shininess, pixel_pos, 0).x;

    vec3 spec_accum = vec3(0);

    ivec2 dims = textureSize(Irradiance, 0);
    float lod_comp_const_part = 0.5 * log2(dims.x * dims.y / float(NumSamples));

    vec3 A;
    if (dot(R, vec3(0, 1, 0)) > 1 - epsilon)
    {
        A = normalize(cross(vec3(1, 0, 0), R));
    }
    else
    {
        A = normalize(cross(vec3(0, 1, 0), R));
    }

    vec3 B = normalize(cross(R, A));

    for (int i = 0; i < NumSamples; ++i)
    {
        vec3 rand = DirFromUV(PhongSkew(rand_vals[i], alpha));
        vec3 L = rand.x * A + rand.y * R + rand.z * B;
        L = normalize(L);

        vec3 H = normalize(V + L);

        float lod = lod_comp_const_part - 0.5 * log2(PhongDist(H, N, alpha)) + 1.0;
        vec3 Reflect_Color = textureLod(Skysphere, DirToUV(L), lod).rgb;
        spec_accum += BRDF_Spec_Monte_Carlo(Reflect_Color, SpecColor, L, V, H, N);
    }

    spec_accum /= NumSamples;

    LightAccumulation += spec_accum;

}
