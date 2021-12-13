#version 450
in vec3 Normal;
in vec3 wvPos;
in vec4 lwvPos;
in vec2 TexCoord;

uniform vec3 light_position;
uniform vec3 view_position;
uniform vec3 light_color;

uniform sampler2D texture_sampler;
uniform sampler2D shadowmap_sampler;
uniform float ambient_light;

out vec4 FragColor;

float GetShadowFactor(vec3 Normal,vec3 LightDir)
{
    vec3 proj=(lwvPos.xyz/lwvPos.w)*.5+.5;
    vec2 uvs=vec2(proj.x,proj.y);
    float cur_d=proj.z;
    float d=texture(shadowmap_sampler,uvs).x;
    float b=max(.0005*(1-dot(Normal,LightDir)),.00005);
    return cur_d-b>d?.2:1;
}

void main(){
    vec3 LightDir=normalize(light_position-wvPos);
    vec3 ViewDir=normalize(view_position-wvPos);
    vec3 ReflectDir=reflect(-LightDir,Normal);
    float ShadowFactor=GetShadowFactor(Normal,LightDir);
    
    float SpecIntensity=.5;
    int shininess=32;
    vec3 AmbientColor=vec3(1,1,1)*ambient_light;
    vec3 DiffuseColor=light_color*max(dot(Normal,LightDir),0);
    vec3 SpecularColor=light_color*SpecIntensity*pow(max(dot(ViewDir,ReflectDir),0),shininess);
    vec4 TextureColor=texture(texture_sampler,TexCoord);
    FragColor=clamp(vec4(AmbientColor+ShadowFactor*(DiffuseColor+SpecularColor),1)*TextureColor,0,1);
}