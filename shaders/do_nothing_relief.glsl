#version 450

layout(quads,equal_spacing,ccw)in;

in vec2 TexCoord[];
in vec3 wvPos[];
in vec3 testPos[];
in vec3 LightPos[];
in vec3 ViewPos[];
in vec4 lwvPos[];

out vec2 TexCoordTe;
out vec3 wvPosTe;
out vec3 testPosTe;
out vec3 LightPosTe;
out vec3 ViewPosTe;
out vec4 lwvPosTe;

vec2 interpolate2(vec2 x1,vec2 x2,vec2 x3,vec2 x4,vec3 uv)
{
    vec2 v1=mix(x1,x2,uv.x);
    vec2 v2=mix(x4,x3,uv.x);
    return mix(v1,v2,uv.y);
}

vec3 interpolate3(vec3 x1,vec3 x2,vec3 x3,vec3 x4,vec3 uv)
{
    vec3 v1=mix(x1,x2,uv.x);
    vec3 v2=mix(x4,x3,uv.x);
    return mix(v1,v2,uv.y);
}

vec4 interpolate4(vec4 x1,vec4 x2,vec4 x3,vec4 x4,vec3 uv)
{
    vec4 v1=mix(x1,x2,uv.x);
    vec4 v2=mix(x4,x3,uv.x);
    return mix(v1,v2,uv.y);
}

void main()
{
    gl_Position=interpolate4(gl_in[0].gl_Position,
        gl_in[1].gl_Position,
        gl_in[2].gl_Position,
        gl_in[3].gl_Position,
    gl_TessCoord);
    
    TexCoordTe=interpolate2(TexCoord[0],TexCoord[1],TexCoord[2],TexCoord[3],gl_TessCoord);
    wvPosTe=interpolate3(wvPos[0],wvPos[1],wvPos[2],wvPos[3],gl_TessCoord);
    testPosTe=interpolate3(testPos[0],testPos[1],testPos[2],testPos[3],gl_TessCoord);
    LightPosTe=interpolate3(LightPos[0],LightPos[1],LightPos[2],LightPos[3],gl_TessCoord);
    ViewPosTe=interpolate3(ViewPos[0],ViewPos[1],ViewPos[2],ViewPos[3],gl_TessCoord);
    lwvPosTe=interpolate4(lwvPos[0],lwvPos[1],lwvPos[2],lwvPos[3],gl_TessCoord);
}