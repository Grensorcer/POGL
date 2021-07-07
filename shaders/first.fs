#version 450
in vec4 Color;
in vec4 LightDir;
in vec2 TexCoord;

uniform sampler2D texture_sampler;
uniform float ambient_light;

out vec4 FragColor;
void main(){
    // FragColor=texture(texture_sampler,TexCoord)*Color;// *ambient_light;
    FragColor=Color;
}