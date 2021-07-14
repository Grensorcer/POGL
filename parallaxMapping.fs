#version 450
in vec2 TexCoord;
in vec3 LightDir;
in vec3 ViewDir;

in vec3 FragPos;
in vec3 TangentLightPos;
in vec3 TangentFragPos;

uniform vec3 light_color;

uniform sampler2D texture_sampler;
uniform sampler2D normal_sampler;
uniform sampler2D height_sampler;
uniform float ambient_light;

out vec4 FragColor;

void main() {
    float numLayer = mix(32, 8, abs(ViewDir.z));  

    float size = 1.0 / numLayer;

    float currLayerDepth = 0.0;

    vec2 P = ViewDir.xy / ViewDir.z * 0.01; 
    vec2 deltaTexCoord = P / numLayer;

    vec2 currCoord = TexCoord;
    float currDepthMapValue = texture(height_sampler, currCoord).x;
      
    while(1 - currLayerDepth > currDepthMapValue)
    {
        currCoord -= deltaTexCoord;
        currDepthMapValue = texture(height_sampler, currCoord).x;
        currLayerDepth += size;  
    }

    vec2 prevTexCoords = currCoord + deltaTexCoord;

    float afterDepth  = currDepthMapValue - currLayerDepth;
    float beforeDepth = texture(height_sampler, prevTexCoords).x - currLayerDepth + size;

    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currCoord * (1.0 - weight);

    if(finalTexCoords.x > 1.0 || finalTexCoords.y > 1.0 || finalTexCoords.x < 0.0 || finalTexCoords.y < 0.0) {
        FragColor = vec4(0, 0, 0, 0);
        return;
    }
        
    vec3 NewNormal = normalize(texture(normal_sampler,finalTexCoords).rgb*2-1);
    vec3 ReflectDir = reflect(-LightDir,NewNormal);

    float SpecIntensity = 0.5;
    int shininess = 32;
    vec3 AmbientColor = vec3(1,1,1) * ambient_light;

    float diff = max(dot(NewNormal, LightDir), 0);
    vec3 DiffuseColor = light_color * diff;

    vec3 SpecularColor = light_color * SpecIntensity * pow(max(dot(ViewDir, ReflectDir),0), shininess);
    vec4 TextureColor = texture(texture_sampler, TexCoord);
    FragColor=clamp(vec4(AmbientColor + DiffuseColor + SpecularColor, 1) * TextureColor, 0, 1);
}