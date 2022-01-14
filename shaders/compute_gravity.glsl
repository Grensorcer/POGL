#version 450
#define MASS.001

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
    int neighbours[];
};

layout(local_size_x=1024)in;

uniform uint nb_vertices;

Vec3 v2V(vec3 v)
{
    Vec3 res;
    res.x=v.x;
    res.y=v.y;
    res.z=v.z;
    return res;
}

vec3 V2v(Vec3 v)
{
    return vec3(v.x,v.y,v.z);
}

vec3 get_neighbour(uint idx,inout float count_neighbours)
{
    int i=neighbours[idx];
    Vec3 res=Vec3(0.,0.,0.);
    
    if(i>=0)
    {
        count_neighbours+=1;
        res=vertices[i];
    }
    
    return V2v(res);
}

void main()
{
    vec3 my_neighbours[4];
    float count_neighbours=0;
    uint idx=gl_GlobalInvocationID.x;
    uint stride=idx*4;
    
    Vec3 Vertex=vertices[idx];
    for(uint i=0;i<4;++i)
    {
        my_neighbours[i]=get_neighbour(stride+i,count_neighbours);
    }
    
    memoryBarrier();
    barrier();
    if(idx<vertices.length())
    {
        vec3 vertex=V2v(Vertex);
        
        vertex=vertex+MASS*vec3(0,-9.81,0);
        vertices[idx]=v2V(vertex);
        // vertices[idx]=v2V(vertex+SCALE*vec3(0,10,0)/count_neighbours);
        // vertices[idx]=v2V((my_neighbours[0]+my_neighbours[1]+my_neighbours[2]+my_neighbours[3])/count_neighbours);
    }
}
