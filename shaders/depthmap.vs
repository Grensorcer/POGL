#version 450
layout(location=0)in vec3 vPosition;
layout(location=1)in vec3 vNormal;
layout(location=2)in vec3 vTangent;
layout(location=3)in vec2 vTexture;

uniform mat4 world;
uniform mat4 wvp;
uniform mat4 light_wvp;
uniform vec3 light_position;
uniform vec3 view_position;

out vec2 TexCoord;
out vec3 wvPos;
out vec3 testPos;
out vec3 LightPos;
out vec3 ViewPos;
out vec4 lwvPos;

mat3 build_w2t()
{
    vec3 T=normalize(vec3(world*vec4(vTangent,0)));
    vec3 N=normalize(vec3(world*vec4(vNormal,0)));
    T=normalize(T-dot(T,N)*N);
    vec3 B=cross(N,T);
    return transpose(mat3(T,B,N));
}

void main(void){
    gl_Position=wvp*vec4(vPosition,1.);
    mat3 w2t=build_w2t();
    wvPos=w2t*vec3(world*vec4(vPosition,1));
    testPos=w2t*vec3(wvp*vec4(vPosition,1));
    lwvPos=light_wvp*vec4(vPosition,1);
    LightPos=w2t*light_position;
    ViewPos=w2t*view_position;
    TexCoord=vTexture;
}