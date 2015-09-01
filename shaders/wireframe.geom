#version 440

layout(triangles) in;
layout(line_strip, max_vertices = 4) out;

in vec3 c_normal[3];
in vec4 c_position[3];

void main()
{
	for (int i = 0; i < 4; ++i)
	{
		gl_Position = gl_in[i % 3].gl_Position;

		EmitVertex();
	}
}
