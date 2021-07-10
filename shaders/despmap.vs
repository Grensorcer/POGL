#version 450

in vec3 position;
in vec2 uv;

out vec2 uv2;
out vec4 LightDir;
out vec3 viewDir;
out vec4 halfwayDir;
out float LightDist;

out vec3 lightColor;
out vec3 positionOut;

vec3 light_color = vec3(1.0, 1.0, 1.0);
vec4 light_position = vec4(1.0, 2.0, 2.0, 1);
vec3 viewPos = vec3(0,0,-17);

mat4 model_view_matrix = mat4(
			      0.57735, -0.33333, 0.57735, 0.00000,
			      0.00000, 0.66667, 0.57735, 0.00000,
			      -0.57735, -0.33333, 0.57735, 0.00000,
			      0.00000, 0.00000, -17, 1.00000);

mat4 projection_matrix = mat4(
			      5.00000, 0.00000, 0.00000, 0.00000,
			      0.00000, 5.00000, 0.00000, 0.00000,
			      0.00000, 0.00000, -1.00020, -1.00000,
			      0.00000, 0.00000, -10.00100, 0.00000);

void main() {
	gl_Position = projection_matrix * model_view_matrix * vec4(position, 1);
	uv2 = uv;
	lightColor = light_color;
	
	LightDir = normalize(light_position - vec4(position, 1));
	viewDir = normalize(viewPos - position);
	halfwayDir = normalize(LightDir + vec4(viewDir,1));
	LightDist = dot(light_position, vec4(position, 1));

	positionOut = position;
}