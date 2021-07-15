#version 450
in vec3 wvPos;
in vec4 lwvPos;
in vec3 LightPos;
in vec3 ViewPos;
in vec2 TexCoord;

uniform vec3 light_color;

uniform sampler2D texture_sampler;
uniform sampler2D normal_sampler;
uniform sampler2D height_sampler;
uniform sampler2D shadowmap_sampler;
uniform float ambient_light;

out vec4 FragColor;

vec2 GetNewUv(vec2 uv,vec3 view_dir,float magic_number)
{
    return uv-(view_dir.xy/view_dir.z)*(1-texture(height_sampler,uv).r)*magic_number;
}

void main(){
    vec3 LightDir=normalize(LightPos-wvPos);
    vec3 ViewDir=normalize(ViewPos-wvPos);
    float magic_number=.05;
    vec2 NewTexCoord=GetNewUv(TexCoord,ViewDir,magic_number);
    /*
    vec4 testcolor=texture(height_sampler,NewTexCoord);
    // bool test=abs(testcolor.r-.1)<.01;
    bool test=testcolor.r>=.4&&testcolor.r<=1.;
    if(test)
    {
        FragColor=vec4(.9,.1,.9,1);
        return;
    }
    */
    
    vec3 NewNormal=normalize(texture(normal_sampler,NewTexCoord).rgb*2-1);
    vec3 ReflectDir=reflect(-LightDir,NewNormal);
    
    float SpecIntensity=.5;
    int shininess=32;
    vec3 AmbientColor=vec3(1,1,1)*ambient_light;
    vec3 DiffuseColor=light_color*max(dot(NewNormal,LightDir),0);
    vec3 SpecularColor=light_color*SpecIntensity*pow(max(dot(ViewDir,ReflectDir),0),shininess);
    vec4 TextureColor=texture(texture_sampler,NewTexCoord);
    FragColor=clamp(vec4(AmbientColor+(DiffuseColor+SpecularColor),1)*TextureColor,0,1);
    //FragColor=vec4(vec3(texture(height_sampler,NewTexCoord).r),1);
}