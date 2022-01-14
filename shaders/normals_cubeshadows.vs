#version 450
layout(location=0)in vec3 vPosition;
layout(location=1)in vec3 vNormal;
layout(location=2)in vec3 vTangent;
layout(location=3)in vec2 vTexture;

uniform mat4 world;
uniform mat4 wvp;

out vec3 Normal;
out vec2 TexCoord;
out vec3 wvPos;

void main(void){
    wvPos=vec3(world*vec4(vPosition,1));
    gl_Position=wvp*vec4(vPosition,1.);
    Normal=normalize(vec3(world*vec4(vNormal,1)));
    TexCoord=vTexture;
}