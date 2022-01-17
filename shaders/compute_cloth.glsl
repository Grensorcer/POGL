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

layout(std430,binding=3)readonly buffer neighbours_buffer
{
    int neighbours[];
};

layout(std430,binding=4)readonly buffer n_vertex_buffer
{
    Vec3 neighbour_vertices[];
};

layout(std430,binding=5)readonly buffer distance_buffer
{
    float distances[];
};

layout(std430,binding=6)buffer info_buffer
{
    ComputeInfo infos[];
};

layout(local_size_x=1024)in;

uniform uint nb_vertices;
uniform vec3 wind;

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
    float mu=7;
    float h=.004;
    
    int m_nidx[8];
    vec3 m_nvec[8];
    float m_ndist[8];
    
    uint count_neighbours=0;
    uint idx=gl_GlobalInvocationID.x;
    uint stride=idx*8;
    
    if(idx<nb_vertices)
    {
        Vec3 Vertex=vertices[idx];
        ComputeInfo info=infos[idx];
        
        for(uint i=0;i<8;++i)
        {
            m_nidx[count_neighbours]=neighbours[stride+i];
            if(m_nidx[count_neighbours]>=0)
            {
                m_nvec[count_neighbours]=V2v(neighbour_vertices[stride+i]);
                m_ndist[count_neighbours]=distances[stride+i];
                count_neighbours+=1;
            }
        }
        
        if(info.pinned==0)
        {
            vec3 vertex=V2v(Vertex);
            
            vec3 force=vec3(0);
            for(uint i=0;i<count_neighbours;++i)
            {
                force+=spring_force(vertex,m_nvec[i],m_ndist[i]);
            }
            force*=K;
            force+=MASS*(vec3(0,-9.81,0)+wind);
            
            vec3 speed=vertex-info.position;
            force-=mu*speed;
            
            vertices[idx]=v2V((h*h*force/MASS)+vertex+speed);
            infos[idx].position=vertex;
        }
    }
}
