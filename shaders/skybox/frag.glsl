#version 330 core

in struct fragment_data
{
	vec3 position;
} fragment;

layout(location=0) out vec4 FragColor;
layout(location=1) out vec4 ExtraColor;
layout(location=2) out vec4 BrightColor;

uniform samplerCube image_skybox;

uniform float direct;
uniform int direct_exp;
uniform float water_surface_plane_length;

uniform vec3 fog_color;
uniform vec3 light_color;
uniform vec3 light_direction;
uniform vec3 camera_position;
uniform vec3 camera_direction;

uniform float fog_distance;

// Main Shader
/***************************************************************************************************/

void main()
{
    // Texture color
    vec3 current_color = vec3(texture(image_skybox, fragment.position));

    // Is this pixel underwater? Since water is transparent, frame
    // can have both underwater and sky pixels at the same time.
    //
    // Two conditions:
    // - Position condition: if player is inside of water, skybox is entirely blue water.
    // - Geometric condition: if z is above a certain threshold value,
    //   that depends on view distance and camera altitude, then player
    //   is looking at sky pixel.
    float water_level = 5.0f;
    vec3 fragment_direction = normalize(fragment.position);
    bool under_water_pixel = camera_position.z < water_level || fragment_direction.z < -camera_position.z / fog_distance;

    // Underwater
    /***********************************************************/
    if (under_water_pixel) {
        current_color = fog_color;
    }
    
    // Over Water Surface
    /***********************************************************/
    else {
    
        // Skybox disappear in fog far away
        vec3 horizon_fog_color  = vec3(1.0f, 1.0f, 1.0f);
        float fog_coefficient   = pow(1 - max(0, fragment_direction.z), 30);
    
        // Direct sunlight
        float direct_magnitude = direct * pow(max(dot(fragment_direction, -light_direction), 0), direct_exp);
        current_color += direct_magnitude * light_color;

        // Horizon fog
        current_color = mix(current_color, horizon_fog_color, fog_coefficient);
    }
    
	// Texture outputs
    /************************************************************/
	FragColor = vec4(current_color, 1.0);
	ExtraColor = vec4(1.0, 1.0, 0.0, 0.0); // Output extra buffers
}