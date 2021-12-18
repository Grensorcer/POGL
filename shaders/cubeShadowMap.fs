#version 450
in vec4 wvPos;

uniform vec3 light_position;
uniform float far_plane;

void main()
{
    // get distance between fragment and light source
    float lightDistance=length(wvPos.xyz-light_position);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance=lightDistance/far_plane;
    
    // write this as modified depth
    gl_FragDepth=lightDistance;
}