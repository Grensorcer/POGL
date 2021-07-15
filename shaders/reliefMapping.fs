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

float GetShadowFactor(vec3 Normal,vec3 LightDir)
{
    vec3 proj=(lwvPos.xyz/lwvPos.w)*.5+.5;
    vec2 uvs=vec2(proj.x,proj.y);
    float cur_d=proj.z;
    float d=texture(shadowmap_sampler,uvs).x;
    float b=max(.0005*(1-dot(Normal,LightDir)),.00005);
    return cur_d-b>d?0:1;
}

float GetDepthIntersection(vec2 TangentDir,vec2 TexCoord)
{
    const int lin_search_steps=10;
    const int bin_search_steps=16;
    float step_depth=1.f/lin_search_steps;
    float step=step_depth;
    float depth=0;
    float best_depth=1;
    
    for(int i=0;i<lin_search_steps;++i)
    {
        depth+=step;
        vec2 uv=TexCoord+depth*TangentDir;
        if(best_depth>=.996&&1-texture(height_sampler,uv).x<=depth)
        {
            best_depth=depth;
        }
    }
    depth=best_depth-step;
    for(int i=0;i<bin_search_steps;++i)
    {
        step*=.5;
        vec2 uv=TexCoord+depth*TangentDir;
        if(depth>=1-texture(height_sampler,uv).x)
        {
            best_depth=depth;
            depth-=2*step;
        }
        depth+=step;
    }
    return best_depth;
}

void main(){
    vec3 LightDir=normalize(LightPos-wvPos);
    vec3 ViewDir=normalize(ViewPos-wvPos);
    float magic_number=.05;
    vec2 TangentViewDir=ViewDir.xy*magic_number/ViewDir.z;
    float BestDepth=GetDepthIntersection(TangentViewDir,TexCoord);
    vec2 NewTexCoord=TexCoord+BestDepth*TangentViewDir;
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
    
    vec2 TangentLightDir=LightDir.xy*magic_number/LightDir.z;
    float LightDepth=GetDepthIntersection(TangentLightDir,NewTexCoord-BestDepth*TangentLightDir);
    float SelfShadowFactor=LightDepth<BestDepth-.05?.3:1;
    
    vec3 NewNormal=normalize(texture(normal_sampler,NewTexCoord).rgb*2-1);
    vec3 ReflectDir=reflect(-LightDir,NewNormal);
    float ShadowFactor=GetShadowFactor(NewNormal,LightDir);
    
    float SpecIntensity=.5;
    int shininess=32;
    vec3 AmbientColor=vec3(1,1,1)*ambient_light;
    vec3 DiffuseColor=light_color*max(dot(NewNormal,LightDir),0);
    vec3 SpecularColor=light_color*SpecIntensity*pow(max(dot(ViewDir,ReflectDir),0),shininess);
    vec4 TextureColor=texture(texture_sampler,NewTexCoord);
    FragColor=clamp(vec4(AmbientColor+ShadowFactor*SelfShadowFactor*(DiffuseColor+SpecularColor),1)*TextureColor,0,1);
    //FragColor=vec4(vec3(texture(height_sampler,NewTexCoord).r),1);
}