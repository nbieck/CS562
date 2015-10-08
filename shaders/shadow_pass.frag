#version 440

in float vtx_depth;

out float depth;

uniform float c;

void main()
{
    depth = exp(c * vtx_depth);
}
