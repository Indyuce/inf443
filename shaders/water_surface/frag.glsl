#version 330 core

// Inputs coming from the vertex shader
in struct fragment_data
{
    vec3 position; // position in the world space
    vec3 normal;   // normal in the world space
    vec3 color;    // current color on the fragment
    vec2 uv;       // current uv-texture on the fragment

} fragment;

// Output of the fragment shader - output color
layout(location=0) out vec4 FragColor;

// View matrix
uniform mat4 view;
uniform vec3 camera_position;
uniform vec3 camera_direction;

struct material_structure
{
	vec3 color;  // Uniform color of the object
};
uniform material_structure material;

// Ambiant uniform controled from the GUI
uniform float ambiant;
uniform float diffuse;
uniform float specular;
uniform int specular_exp;

uniform float flashlight;
uniform int flashlight_exp;
uniform float flashlight_dist;
uniform vec3 flashlight_color;

uniform vec3 light_color;
uniform vec3 light_direction;

uniform vec3 fog_color;
uniform bool surf_height;
uniform float floor_level;
uniform float scale;
uniform float water_attenuation_coefficient;
uniform float water_optical_index;

struct gerstner_wave {
    vec2 direction;
    float amplitude;
    float steepness;
    float frequency;
    float speed;
};

uniform samplerCube image_skybox;
uniform sampler2D texture_sand;
uniform float sand_texture_scale;

void main()
{
    vec3 current_color;

    // Height map shader for debug
    /***********************************************************/
    if (surf_height) {
        float p = fragment.position.z;
        vec3 color1 = vec3(0.012,0.478,0.871);
        vec3 color2 = vec3(0.012,0.898,0.718);
        current_color = p * color2 + (1 - p) * color1;
        FragColor = vec4(current_color, 1.0);
        return;
    }

    // Prepare for refraction/reflection
    vec3 N = fragment.normal;
    float attenuation_distance = 1.0f;
    float eta = water_optical_index;
    float height = abs(floor_level);
    vec3 I = normalize(fragment.position - camera_position);

    // UNDERWATER
    if (gl_FrontFacing == false) {
		N = -N;
        attenuation_distance = length(fragment.position - camera_position);
        vec3 texture_coords = refract(I, N, eta);

        // Total reflection
        if (texture_coords.x == 0 && texture_coords.y == 0 && texture_coords.z == 0)
            current_color = texture(texture_sand, reflect(I, N).xy).xyz;

        // Inside of Snell's window!!!
        else
            current_color = texture(image_skybox, texture_coords).xyz;
            
        // Color attenuation
        float attenuation = exp(-water_attenuation_coefficient * scale * attenuation_distance);
        current_color = current_color * attenuation + (1 - attenuation) * fog_color;
	}
    
    // ABOVE SURFACE LEVEL
    else {
        eta = 1.0f / eta;
        // TODO color attenuation  ????
        attenuation_distance = height / abs(fragment.position.z);

        // Partial reflection
        // Water reflects at normal angles and refracts more at steep angles
        float angle_steepness = max(0, dot(vec3(0, 0, 1), -I));
        
        vec3 refracted_color = texture(texture_sand, refract(I, N, eta).xy).xyz;
        vec3 reflected_color = texture(image_skybox, reflect(I, N)).xyz;
        current_color = mix(refracted_color, reflected_color, 1 - angle_steepness);
    }

    // Specular sunlight
    // TODO
    // float specular_magnitude = pow(max(dot(R, Cn), 0.0), specularExp) * specular;
    // current_color += specular_magnitude * light_color;

    FragColor = vec4(current_color, 1.0); // Note: the last alpha component is not used here
}
