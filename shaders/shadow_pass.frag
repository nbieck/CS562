#version 440

in float vtx_depth;

out float depth;

void main()
{
    depth = vtx_depth;
}
