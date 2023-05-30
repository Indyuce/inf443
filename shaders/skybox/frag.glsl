#version 330 core

in struct fragment_data
{
	vec3 position;
} fragment;

layout(location=0) out vec4 FragColor;

uniform samplerCube image_skybox;

uniform float direct;
uniform int direct_exp;
uniform float water_surface_plane_length;

uniform vec3 fog_color;
uniform vec3 light_color;
uniform vec3 light_direction;
uniform vec3 camera_position;
uniform vec3 camera_direction;

void main()
{
    // Texture color
    vec3 current_color = vec3(texture(image_skybox, fragment.position));

    // Underwater
    bool under_water = camera_position.z < 5f;
    /***********************************************************/
    if (under_water) {
        current_color = fog_color;
    }
    
    // Over Water Surface
    /***********************************************************/
    else {
    
        // Skybox disappear in fog far away
        vec3 horizon_fog_color  = vec3(1.0f, 1.0f, 1.0f);
        vec3 fragment_direction = normalize(fragment.position);
        float fog_coefficient   = pow(1 - max(0, fragment_direction.z), 30);
    
        // Direct sunlight
        float direct_magnitude = direct * pow(max(dot(fragment_direction, -light_direction), 0), direct_exp);
        current_color += direct_magnitude * light_color;

        // Horizon fog
        current_color = mix(current_color, horizon_fog_color, fog_coefficient);
    }

	FragColor = vec4(current_color, 1.0);
}