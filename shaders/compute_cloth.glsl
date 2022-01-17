#version 450
#define MASS.02
#define R.02

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

layout(std430,binding=2)readonly buffer normal_buffer
{
    Vec3 normals[];
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

vec3 spring_force(vec3 u,vec3 v,float L0)
{
    vec3 diff=v-u;
    float d=length(diff);
    return(d-L0)*diff/d;
}

vec3 collide(in vec3 cloth_particle,in vec3 sphere,in vec3 sphere_normal)
{
    vec3 diff=cloth_particle-sphere;
    float collision=length(diff);
    
    vec3 res=vec3(0);
    if(collision<=2*R)
    {
        vec3 dir=normalize(diff);
        float d=2*R-collision;
        res+=(d/2)*dir;
    }
    
    return res;
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
        vec3 vertex=V2v(Vertex);
        ComputeInfo info=infos[idx];
        
        if(info.pinned==0)
        {
            // Self-Collision
            vec3 collision_dir=vec3(0);
            uint count_collisions=0;
            for(uint i=0;i<idx;++i)
            {
                vec3 dir=collide(vertex,V2v(vertices[i]),V2v(normals[i]));
                collision_dir+=dir;
                if(dir!=vec3(0))
                {
                    count_collisions+=1;
                }
            }
            
            for(uint i=idx+1;i<nb_vertices;++i)
            {
                vec3 dir=collide(vertex,V2v(vertices[i]),V2v(normals[i]));
                collision_dir+=dir;
                if(dir!=vec3(0))
                {
                    count_collisions+=1;
                }
            }
            vertex+=count_collisions>0?collision_dir/count_collisions:collision_dir;
            
            // Get Neighbours
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
            
            // Compute forces
            vec3 force=vec3(0);
            for(uint i=0;i<count_neighbours;++i)
            {
                force+=spring_force(vertex,m_nvec[i],m_ndist[i]);
            }
            force*=K;
            force+=MASS*(vec3(0,-9.81,0)+wind);
            
            vec3 speed=vertex-info.position;
            force-=mu*speed;
            
            // Verlet-Integration
            vertices[idx]=v2V((h*h*force/MASS)+vertex+speed);
            infos[idx].position=vertex;
        }
    }
}
