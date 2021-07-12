#version 450
in vec2 TexCoord;

uniform vec3 light_color;

uniform sampler2D texture_sampler;
uniform float ambient_light;

out vec4 FragColor;
void main(){
    vec3 AmbientColor=vec3(1,1,1)*.5;
    vec4 TextureColor=texture(texture_sampler,TexCoord);
    FragColor=clamp(vec4(AmbientColor,1)*TextureColor,0,1);
}