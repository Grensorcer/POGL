#version 450
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 wvp[6];

out vec4 wvPos;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i)
        {
            wvPos = gl_in[i].gl_Position;
            gl_Position = wvp[face] * wvPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}  