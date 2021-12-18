#version 450
#define SCALE.001

struct Vec3{
    float x,y,z;
};

layout(std430,binding=1)buffer vertex_buffer
{
    Vec3 vertices[];
};

layout(std430,binding=2)buffer normal_buffer
{
    Vec3 normals[];
};

layout(std430,binding=3)buffer neighbours_buffer
{
    uint neighbours[];
};

layout(local_size_x=1024)in;

uniform uint nb_vertices;

void main()
{
    uint idx=gl_GlobalInvocationID.x;
    Vec3 v=vertices[idx];
    vec3 vertex=vec3(v.x,v.y,v.z);
    if(idx<nb_vertices)
    {
        vertex=vertex+SCALE*vec3(0,-9.81,0);
        vertices[idx].x=vertex.x;
        vertices[idx].y=vertex.y;
        vertices[idx].z=vertex.z;
    }
}
