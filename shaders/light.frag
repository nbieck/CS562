#version 440

in vec3 c_normal;
in vec4 c_position;

out vec4 color;

uniform vec4 c_light_position;

struct LightModelParams
{
	vec4 ambient;
};

LightModelParams LightModel = { vec4(0.1) };

struct MaterialParams
{
	vec4 emission;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

MaterialParams Material = {
vec4(0),
vec4(0.1745, 0.01175, 0.01175, 1),
vec4(0.61424, 0.04136, 0.04136, 1),
vec4(0.727811, 0.626959, 0.626959, 1),
76.8 };

struct LightSourceParams
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float const_att;
	float linear_att;
	float quad_att;
};

LightSourceParams LightSource = {
vec4(1),
vec4(1),
vec4(1),
1.0,
0.0,
1.0/250.0 };

struct FogParams
{
	vec4 color;
	float far;
	float near;
};

FogParams Fog = {
vec4(0),
40.0,
0.1 };

vec4 Ambient()
{
	return LightSource.ambient * Material.ambient;
}

vec4 Diffuse(vec3 L)
{
	return LightSource.diffuse * Material.diffuse * max(dot(L, normalize(c_normal)), 0);
}

vec4 Specular(vec3 R, vec3 V)
{
	return LightSource.specular * Material.specular * pow(max(dot(R, V), 0), Material.shininess);
}

float Attenuation()
{
	float d = length(c_light_position - c_position);

	return min(1.0 / (LightSource.const_att + LightSource.linear_att * d + LightSource.quad_att * d * d), 1.0);
}

void main()
{
	vec4 colorAcc = vec4(0);
    
    vec3 L;
    
    L = (c_light_position - c_position).xyz;
    L = normalize(L);
    
    vec3 R = reflect(-L, normalize(c_normal));

    vec3 V = -c_position.xyz;
    V = normalize(V);

    float att = Attenuation();

    colorAcc += Ambient() * att;
    colorAcc += Diffuse(L) * att;
    colorAcc += Specular(R, V) * att;

	colorAcc = colorAcc + Material.emission + Material.ambient * LightModel.ambient;

	float fog_interpolator = (Fog.far + c_position.z) / (Fog.far - Fog.near);

	color = fog_interpolator * colorAcc + (1 - fog_interpolator) * Fog.color;
}
