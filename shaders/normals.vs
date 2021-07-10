#version 450
layout(location=0)in vec3 vPosition;
layout(location=1)in vec3 vNormal;
layout(location=2)in vec3 vTangent;
layout(location=3)in vec2 vTexture;

uniform mat4 world;
uniform mat4 wvp;
uniform vec3 light_position;
uniform vec3 view_position;

out vec3 Normal;
out vec2 TexCoord;
out vec3 LightDir;
out vec3 ViewDir;

void main(void){
    vec3 wvPos=vec3(world*vec4(vPosition,1));
    gl_Position=wvp*vec4(vPosition,1.);
    Normal=normalize(vec3(world*vec4(vNormal,1)));
    LightDir=normalize(light_position-wvPos);
    ViewDir=normalize(view_position-wvPos);
    TexCoord=vTexture;
}