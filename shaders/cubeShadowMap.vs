#version 450
layout(location=0)in vec3 vPosition;

uniform mat4 world;

void main()
{
    gl_Position=world*vec4(vPosition,1.);
}