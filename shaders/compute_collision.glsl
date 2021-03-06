#version 450
#define R_cloth.02
#define R_static.05

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

layout(std430,binding=7)buffer collision_buffer
{
    Vec3 collisions[];
};

layout(std430,binding=8)buffer normal_buffer
{
    Vec3 normals[];
};

layout(local_size_x=1024)in;

uniform uint nb_vertices;
uniform uint nb_collisions;
uniform mat4 world;
uniform mat4 collision_world;

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

vec3 collide(in vec3 cloth_particle,in vec3 sphere,in vec3 sphere_normal)
{
    vec3 diff=cloth_particle-(sphere-sphere_normal*R_static/2);
    float collision=length(diff);
    
    vec3 res=vec3(0);
    if(collision<=R_cloth+R_static)
    {
        vec3 dir=normalize(diff);
        float d=R_cloth+R_static-collision;
        res+=(d/2)*dir;
    }
    
    return res;
}

void main()
{
    uint idx=gl_GlobalInvocationID.x;
    
    Vec3 Vertex=vertices[idx];
    
    if(idx<vertices.length())
    {
        vec3 vertex=V2v(vertices[idx]);
        vec3 w_vertex=(world*vec4(vertex,1.)).xyz;
        
        vec3 approximation=vec3(0);
        int collision_count=0;
        for(uint i=0;i<collisions.length();++i)
        {
            vec3 collision=V2v(collisions[i]);
            vec3 normal=V2v(normals[i]);
            vec3 res=collide(w_vertex,(collision_world*vec4(collision,1.)).xyz,(collision_world*vec4(normal,1.)).xyz);
            if(res!=vec3(0))
            {
                collision_count+=1;
                approximation+=res;
            }
        }
        
        approximation=(collision_count==0)?approximation:approximation/collision_count;
        
        vertices[idx]=v2V(vertex+approximation);
    }
}
