#version 450
layout(location=0)in vec3 vPosition;
layout(location=1)in vec3 vNormal;
layout(location=2)in vec3 vTangent;
layout(location=3)in vec2 vTexture;

uniform mat4 world;
uniform mat4 wvp;
uniform vec3 light_position;
uniform vec3 view_position;

out VS_OUT {
    vec2 TexCoord;
    vec3 LightDir;
    vec3 ViewDir;

    vec3 FragPos;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

mat3 build_w2t()
{
    vec3 T=normalize(vec3(world*vec4(vTangent,0)));
    vec3 N=normalize(vec3(world*vec4(vNormal,0)));
    T=normalize(T-dot(T,N)*N);
    vec3 B=cross(N,T);
    return transpose(mat3(T,B,N));
}

void main(void){
    mat3 w2t=build_w2t();
    
    vs_out.FragPos   = vec3(world * vec4(vPosition, 1.0)); 

    vs_out.TangentLightPos = w2t * light_position;
    vs_out.TangentViewPos  = w2t * wvp;
    vs_out.TangentFragPos  = w2t * vs_out.FragPos;

    vec3 wvPos=vec3(world*vec4(vPosition,1));
    gl_Position=wvp*vec4(vPosition,1.);
    vs_out.LightDir=w2t*normalize(light_position-wvPos);
    vs_out.ViewDir=w2t*normalize(view_position-wvPos);
    vs_out.TexCoord=vTexture;
}