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
    
    // Is the player looking at a water pixels
    // TODO
    vec2 projected = camera_direction.xy * abs(camera_position.z / camera_direction.z);

    // Underwater
    bool under_water = camera_position.z < 5f;
    /***********************************************************/
    if (under_water) {
        current_color = fog_color;
    }
    
    // Over Water Surface
    /***********************************************************/
    else {
    
        // Direct sunlight
        vec3 fragment_direction = normalize(fragment.position);
        float direct_magnitude = direct * pow(max(dot(fragment_direction, -light_direction), 0), direct_exp);
        current_color += direct_magnitude * light_color;
    }


	FragColor = vec4(current_color, 1.0);
}