#version 440

#define PI 3.1415926535897932384626433832795

in vec3 c_normal;
in vec4 c_position;

out vec4 color;

uniform vec4 c_light_position;

struct MaterialParams
{
	vec4 F_0;
	float roughness;
};

uniform MaterialParams Material;

struct LightSourceParams
{
	vec4 color;
};

LightSourceParams LightSource = {
vec4(0.3)};

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

vec4 SchlickFresnel(vec4 f_0, vec3 light_dir, vec3 half_vec)
{
	return f_0 + (1 - f_0) * pow((1 - dot(light_dir, half_vec)), 5);
}

float DistributionFunc(vec3 dir, vec3 normal)
{
	//use Beckmann distribution
	float cos_a = dot(dir, normal);

	return (exp(-((1 - cos_a * cos_a) / (cos_a * cos_a * Material.roughness))))
		/ (PI * Material.roughness * Material.roughness * cos_a * cos_a * cos_a * cos_a);
}

float GeometryFunction(vec3 light, vec3 view, vec3 half_vec, vec3 normal)
{
	float term_1 = (2 * dot(half_vec, normal) * dot(view, normal)) / dot(view, half_vec);
	float term_2 = (2 * dot(half_vec, normal) * dot(light, normal)) / dot(view, half_vec);

	//float result = min(term_1, term_2);
	float result = min(term_1, 1);

	return 1;

	//return result;
}

vec4 Diffuse(vec3 L, vec3 normal)
{
	return LightSource.color * Material.F_0 * max(dot(L, normal), 0) * 0.5;
}

void main()
{
	vec3 view = -c_position.xyz;
	view = normalize(view);
	vec3 light = (c_light_position - c_position).xyz;
	light = normalize(light);
	vec3 half_vec = normalize(view + light);

	vec3 normal = normalize(c_normal);
	if (!gl_FrontFacing)
		normal = -normal;

	vec4 light_col = LightSource.color * max(0, dot(normal, light));

	color = SchlickFresnel(Material.F_0, light, half_vec) * light_col;
	color *= DistributionFunc(half_vec, normal);
	color *= GeometryFunction(light, view, half_vec, normal);

	color /= 4 * dot(view, normal) * dot(normal, light);

	color += Diffuse(light, normal);
}
