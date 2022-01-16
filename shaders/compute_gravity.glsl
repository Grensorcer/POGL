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

layout(std430,binding=4)readonly buffer distance_buffer
{
    float distances[];
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

vec3 spring_force(vec3 u,vec3 v,float L0)
{
    vec3 diff=v-u;
    float d=length(diff);
    return(d-L0)*diff/d;
}

void main()
{
    float K=500;
    float mu=10;
    float h=.004;
    
    int m_nidx[8];
    vec3 m_nvec[8];
    float m_ndist[8];
    
    uint count_neighbours=0;
    uint idx=gl_GlobalInvocationID.x;
    uint stride=idx*8;
    
    Vec3 Vertex=vertices[idx];
    ComputeInfo info=infos[idx];
    
    for(uint i=0;i<8;++i)
    {
        uint j=count_neighbours;
        m_nidx[j]=get_neighbour(i,stride,count_neighbours);
        m_ndist[j]=distances[stride+i];
    }
    for(uint i=0;i<count_neighbours;++i)
    {
        m_nvec[i]=V2v(vertices[m_nidx[i]]);
    }
    
    memoryBarrier();
    barrier();
    
    if((idx<nb_vertices)&&(info.pinned==0))
    {
        vec3 vertex=V2v(Vertex);
        vec3 force=vec3(0);
        
        for(uint i=0;i<count_neighbours;++i)
        {
            force+=spring_force(vertex,m_nvec[i],m_ndist[i]);
        }
        force*=K;
        force+=MASS*vec3(0,-9.81,0);
        
        vec3 speed=vertex-info.position;
        force-=mu*speed;
        
        vertices[idx]=v2V((h*h*force/MASS)+vertex+speed);
        infos[idx].position=vertex;
    }
    
    memoryBarrier();
    barrier();
    
    for(uint i=0;i<count_neighbours;++i)
    {
        m_nvec[i]=V2v(vertices[m_nidx[i]]);
    }
    
    if((idx<nb_vertices)&&(info.pinned==0))
    {
        vec3 vertex=V2v(vertices[idx]);
        vec3 normal=vec3(0);
        for(uint i=0;i<count_neighbours;++i)
        {
            vec3 p1=V2v(vertices[m_nidx[i]]);
            vec3 p2=V2v(vertices[m_nidx[(i+1)%count_neighbours]]);
            normal+=normalize(cross(p1-vertex,p2-vertex));
        }
        normal/=count_neighbours;
        normals[idx]=v2V(normal);
    }
}
