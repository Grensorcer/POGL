#version 450
layout(location=0)in vec3 vPosition;
layout(location=3)in vec2 vTexture;

uniform mat4 light_wvp;

out vec2 TexCoord;
void main(void){
    gl_Position=light_wvp*vec4(vPosition,1.);
    TexCoord=vTexture;
}