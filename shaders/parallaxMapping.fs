#version 450
in VS_OUT {
    vec2 TexCoord;
    vec3 LightDir;
    vec3 ViewDir;

    vec3 FragPos;
    vec3 TangentLightPos;
    vec3 TangentFragPos;
} fs_in;

uniform vec3 light_color;

uniform sampler2D texture_sampler;
uniform sampler2D normal_sampler;
uniform sampler2D height_sampler;
uniform float ambient_light;

out vec4 FragColor;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir){
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    float heightScale = 0.01;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(height_sampler, currentTexCoords).r;
      
    while(1 - currentLayerDepth > currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get height_sampler value at current texture coordinates
        currentDepthMapValue = texture(height_sampler, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(height_sampler, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main() {
    /*
    vec4 tmp = texture(height_sampler, fs_in.TexCoord);
    if (tmp.r <= 0.3 && tmp.r >= 0) {
        FragColor = vec4(1,0,0,1);
        return;
    }*/


    vec3 LightDir = fs_in.LightDir;

    vec2 texCoords = ParallaxMapping(fs_in.TexCoord,  fs_in.ViewDir);       
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        FragColor = vec4(0, 0, 0, 0);
    else {
        vec3 NewNormal = normalize(texture(normal_sampler,texCoords).rgb*2-1);
        vec3 ReflectDir = reflect(-LightDir,NewNormal);

        float SpecIntensity = 0.5;
        int shininess = 32;
        vec3 AmbientColor = vec3(1,1,1) * ambient_light;

        float diff = max(dot(NewNormal, LightDir), 0);
        vec3 DiffuseColor = light_color * diff;

        vec3 SpecularColor = light_color * SpecIntensity * pow(max(dot(fs_in.ViewDir, ReflectDir),0), shininess);
        vec4 TextureColor = texture(texture_sampler, fs_in.TexCoord);
        FragColor=clamp(vec4(AmbientColor + DiffuseColor + SpecularColor, 1) * TextureColor, 0, 1);
    }
}