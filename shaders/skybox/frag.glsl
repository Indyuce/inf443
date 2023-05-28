#version 330 core

in struct fragment_data
{
	vec3 position;
} fragment;

layout(location=0) out vec4 FragColor;

uniform samplerCube image_skybox;

uniform float direct;
uniform int direct_exp;

uniform vec3 light_color;
uniform vec3 light_direction;
uniform vec3 camera_position;
uniform vec3 camera_direction;

void main()
{
    // Texture color
    vec3 current_color = vec3(texture(image_skybox, fragment.position));

    // Underwater
    vec3 sky_blue = vec3(0.529,0.808,0.922);
    float alpha = 0.12; // Water attenuation coefficient at 350nm
    float scale = 10.0f; // Scale correction coefficient
   // if (fragment.position.z < 0) {
   //     current_color = sky_blue * exp(-alpha * scale * abs(normalize(fragment.position).z));
   // }

    // Direct sunlight
    vec3 fragment_direction = normalize(fragment.position);
    float direct_magnitude = direct * pow(max(dot(fragment_direction, -light_direction), 0), direct_exp);
    current_color += direct_magnitude * light_color;

	FragColor = vec4(current_color, 1.0);
}