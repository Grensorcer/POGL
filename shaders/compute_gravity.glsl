#version 450
#define MASS.02

struct Vec3{
    float x,y,z;
};

struct ComputeInfo{
    vec3 speed;
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

vec3 get_neighbour(uint idx,inout uint count_neighbours)
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

vec3 spring_force(vec3 u,vec3 v,float L0)
{
    vec3 diff=v-u;
    float d=length(diff);
    return(d-L0)*diff/d;
}

void main()
{
    float K=200;
    float mu=.2;
    float h=.003;
    
    vec3 my_neighbours[8];
    float my_distances[8];
    
    uint count_neighbours=0;
    uint idx=gl_GlobalInvocationID.x;
    uint stride=idx*8;
    
    Vec3 Vertex=vertices[idx];
    ComputeInfo info=infos[idx];
    for(uint i=0;i<8;++i)
    {
        my_neighbours[i]=get_neighbour(stride+i,count_neighbours);
        my_distances[i]=distances[stride+i];
    }
    
    memoryBarrier();
    barrier();
    
    if((idx<nb_vertices)&&(info.pinned==0))
    {
        vec3 vertex=V2v(Vertex);
        
        vec3 force=vec3(0);
        for(uint i=0;i<count_neighbours;++i)
        {
            force+=spring_force(vertex,my_neighbours[i],my_distances[i]);
        }
        force*=K;
        force+=MASS*vec3(0,-9.81,0);
        force-=mu*info.speed;
        
        infos[idx].speed+=h*force/MASS;
        vertex+=h*infos[idx].speed;
        //vertex+=h*force;
        vertices[idx]=v2V(vertex);
        // vertices[idx]=v2V(vertex+SCALE*vec3(0,10,0)/count_neighbours);
        // vertices[idx]=v2V((my_neighbours[0]+my_neighbours[1]+my_neighbours[2]+my_neighbours[3])/count_neighbours);
    }
}
