#version 450

in vec2 uv2;
in vec4 LightDir;
in vec3 lightColor;
in vec3 viewDir;
in vec3 positionOut;
in vec4 halfwayDir;
in float LightDist;

uniform sampler2D texture_sampler;
// DESP map
uniform sampler2D lighting_sampler;
uniform sampler2D normalmap_sampler;

layout(location=0) out vec4 output_color;


float get_depht(vec2 dp, vec2 ds) {
    int linear_search_steps = 10;
    int binary_search_steps = 5;
    float depth_step = 1.0;
    float size = depth_step;
    float depth = 0.0;
    float best_depth = 1.0;

    

    for (int i = 0; i < linear_search_steps; i++) {
        depth += size;
        vec4 t = texture(lighting_sampler, dp + ds * depth);
        float gray = (t.x + t.y + t.z) / 3;
        // if (best_depth > 0.996) attention
        if (depth >= gray) {
            best_depth = depth;
            break;
        }
    }

    depth = best_depth;
    for (int i = 0; i < binary_search_steps; i++) {
        size *= 0.5;
        vec4 t = texture(lighting_sampler, dp + ds * depth);
        float gray = (t.x + t.y + t.z) / 3;
        if (depth >= gray) {
            best_depth = depth;
            depth -= 2*size;
        }
        depth += size;
    }
    return best_depth;
}


void main() {
    // get Normal
    vec3 normal = texture(normalmap_sampler, uv2).rgb;
    normal = normalize(normal * 2.0 - 1.0);


    // Blinn-Phong
    float Blinn_Phong_spec = pow(max(dot(vec4(normal,1), halfwayDir), 0.0), 2);

    /*
    float depth = 1;
    vec2 ds = vec2(viewDir.x, viewDir.y) * depth / viewDir.z;
    vec2 dp = uv2;


    float d = get_depht(dp, ds);
    vec2 uv = dp + ds * d;

    vec4 t = texture(lighting_sampler, uv);
    */

        
    //positionOut += normalize(positionOut) * d * viewDir.z;

    // get Texture
    vec4 color = texture(texture_sampler, uv2);
    color = vec4(0,0,0.5,1);

    // Get diffuse
    float diff = max(dot(normal, LightDir.rgb), 0.0);
    vec3 diffuse = diff * lightColor;

    // SpecularStrength
    float specularStrength = 0.5;

    vec4 reflectDir = reflect(-LightDir, vec4(normal, 1));

    // get Specular
    float spec = pow(max(dot(vec4(viewDir, 1), reflectDir), 0.0), 2);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 tmp = diffuse + Blinn_Phong_spec;
    // with Texture
    output_color = color * (vec4(tmp, 1));

    // With blue
    //output_color = vec4(0, 0, 0.4, 0) * (vec4(tmp, 1));

    // gamma correction
    float gamma = 1.1;
    //gamma = LightDist;
    output_color = pow(output_color, vec4(1.0/gamma));

}
