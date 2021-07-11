#version 450

in vec2 TexCoord;

uniform sampler2D shadowmap_sampler;

out vec4 FragColor;

void main(){
    float DepthValue=texture(shadowmap_sampler,TexCoord).r;
    FragColor=vec4(vec3(DepthValue),1);
}
