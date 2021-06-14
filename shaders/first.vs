#version 450
layout(location=0)in vec3 vPosition;
layout(location=1)in vec3 vColor;
uniform mat4 projection_matrix;
out vec4 color;
void main(void){
    gl_Position=projection_matrix*vec4(vPosition,1.);
    color=vec4(vColor,1.);
}