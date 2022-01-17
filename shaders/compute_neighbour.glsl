#version 450

struct Vec3{
    float x,y,z;
};

struct ComputeInfo{
    vec3 position;
    float pinned;
};

layout(std430,binding=1)readonly buffer vertex_buffer
{
    Vec3 vertices[];
};

layout(std430,binding=3)readonly buffer neighbours_buffer
{
    int neighbours[];
};

layout(std430,binding=4)buffer n_vertex_buffer
{
    Vec3 neighbour_vertices[];
};

layout(local_size_x=1024)in;

uniform uint nb_vertices;

void main()
{
    uint idx=gl_GlobalInvocationID.x;
    
    if(idx<nb_vertices)
    {
        Vec3 me=vertices[idx];
        for(uint i=0;i<neighbours.length();++i)
        {
            if(neighbours[i]==idx)
            {
                neighbour_vertices[i]=me;
            }
        }
    }
}
