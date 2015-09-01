#version 440

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 ViewProj;

void main()
{
	vec3 pos = gl_in[0].gl_Position.xyz;
	vec3 normal = normalize(gl_in[1].gl_Position.xyz);

	vec3 helper_axis = vec3(0,1,0);
	if (length(normal - helper_axis) < 0.0001)
		helper_axis = vec3(1,0,0);

	vec3 tangent_1 = normalize(cross(normal, helper_axis));
	vec3 tangent_2 = normalize(cross(normal, tangent_1));

	gl_Position = ViewProj * vec4(pos + tangent_1 * 3, 1);
	EmitVertex();
	gl_Position = ViewProj * vec4(pos + tangent_2 * 3, 1);
	EmitVertex();
	gl_Position = ViewProj * vec4(pos - tangent_2 * 3, 1);
	EmitVertex();
	gl_Position = ViewProj * vec4(pos - tangent_1 * 3, 1);
	EmitVertex();
}
