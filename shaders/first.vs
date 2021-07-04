#version 450
layout(location=0)in vec3 vPosition;
layout(location=1)in vec3 vColor;
layout(location=2)in vec2 vTexture;

uniform mat4 projection_matrix;
uniform vec3 light_position;
uniform vec3 light_color;

out vec4 Color;
out vec4 LightDir;
out vec2 TexCoord;

void main(void){
    gl_Position=projection_matrix*vec4(vPosition,1.);
    Color=vec4(vColor,1.);
    LightDir=vec4(normalize(light_position-vPosition),1.);
    TexCoord=vTexture;
}