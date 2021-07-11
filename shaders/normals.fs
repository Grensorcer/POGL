#version 450
in vec3 Normal;
in vec3 wvPos;
in vec2 TexCoord;

uniform vec3 light_position;
uniform vec3 view_position;
uniform vec3 light_color;

uniform sampler2D texture_sampler;
uniform float ambient_light;

out vec4 FragColor;
void main(){
    vec3 LightDir=normalize(light_position-wvPos);
    vec3 ViewDir=normalize(view_position-wvPos);
    vec3 ReflectDir=reflect(-LightDir,Normal);
    
    float SpecIntensity=.5;
    int shininess=32;
    vec3 AmbientColor=vec3(1,1,1)*ambient_light;
    vec3 DiffuseColor=light_color*max(dot(Normal,LightDir),0);
    vec3 SpecularColor=light_color*SpecIntensity*pow(max(dot(ViewDir,ReflectDir),0),shininess);
    vec4 TextureColor=texture(texture_sampler,TexCoord);
    FragColor=clamp(vec4(AmbientColor+DiffuseColor+SpecularColor,1)*TextureColor,0,1);
}