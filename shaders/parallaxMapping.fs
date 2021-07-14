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

    float layer = 0.0;

    vec2 P = ViewDir.xy / ViewDir.z * 0.01; 
    vec2 d = P / numLayer;
    vec2 uv = TexCoord;

    while(1 - layer > texture(height_sampler, uv).x)
    {
        uv -= d;
        layer += size;  
    }

    vec2 uv2 = uv + d;
    float afterDepth  = texture(height_sampler, uv).x - layer;
    float beforeDepth = texture(height_sampler, uv2).x - layer + size;
    float weight = afterDepth / (afterDepth - beforeDepth);

    vec2 finalUV = uv2 * weight + uv * (1.0 - weight);
    if(finalUV.x > 1.0 || finalUV.y > 1.0 || finalUV.x < 0.0 || finalUV.y < 0.0) {
        FragColor = vec4(0, 0, 0, 0);
        return;
    }

    vec3 NewNormal = normalize(texture(normal_sampler,finalUV).rgb*2-1);
    vec3 ReflectDir = reflect(-LightDir,NewNormal);

    float SpecIntensity = 0.5;
    int shininess = 32;
    vec3 AmbientColor = vec3(1,1,1) * ambient_light;

    float diff = max(dot(NewNormal, LightDir), 0);
    vec3 DiffuseColor = light_color * diff;

    vec3 SpecularColor = light_color * SpecIntensity * pow(max(dot(ViewDir, ReflectDir),0), shininess);
    vec4 TextureColor = texture(texture_sampler, finalUV);
    FragColor=clamp(vec4(AmbientColor + DiffuseColor + SpecularColor, 1) * TextureColor, 0, 1);
}