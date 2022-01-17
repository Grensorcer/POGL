#version 450
#define MASS.02

struct Vec3{
    float x,y,z;
};

struct ComputeInfo{
    vec3 position;
    float pinned;
};

layout(std430,binding=1)buffer vertex_buffer
{
    Vec3 vertices[];
};

layout(std430,binding=2)buffer normal_buffer
{
    Vec3 normals[];
};

layout(std430,binding=3)readonly buffer neighbours_buffer
{
    int neighbours[];
};

layout(std430,binding=5)buffer info_buffer
{
    ComputeInfo infos[];
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

int get_neighbour(uint idx,uint stride,inout uint count_neighbours)
{
    int i=neighbours[stride+idx];
    
    if(i>=0)
    {
        count_neighbours+=1;
    }
    
    return i;
}

void main()
{
    uint count_neighbours=0;
    uint idx=gl_GlobalInvocationID.x;
    uint stride=idx*8;
    
    if(idx<nb_vertices)
    {
        int m_nidx[8];
        vec3 m_nvec[8];
        
        for(uint i=0;i<8;++i)
        {
            uint j=count_neighbours;
            m_nidx[j]=get_neighbour(i,stride,count_neighbours);
        }
        for(uint i=0;i<count_neighbours;++i)
        {
            m_nvec[i]=V2v(vertices[m_nidx[i]]);
        }
        
        vec3 vertex=V2v(vertices[idx]);
        vec3 normal=vec3(0);
        for(uint i=0;i<count_neighbours;++i)
        {
            vec3 p1=V2v(vertices[m_nidx[i]]);
            vec3 p2=V2v(vertices[m_nidx[(i+1)%count_neighbours]]);
            normal+=cross(p1-vertex,p2-vertex);
        }
        normal/=count_neighbours;
        normals[idx]=v2V(normalize(normal));
    }
}
