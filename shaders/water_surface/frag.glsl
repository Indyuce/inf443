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
uniform int specularExp;

uniform float flashlight;
uniform int flashlight_exp;
uniform float flashlight_dist;
uniform vec3 flashlight_color;

uniform vec3 light_color;
uniform vec3 light_direction;

uniform vec3 fog_color;
uniform float fog_distance;
// uniform float attenuation_distance;
uniform bool surf_height;

struct gerstner_wave {
    vec2 direction;
    float amplitude;
    float steepness;
    float frequency;
    float speed;
};

uniform samplerCube image_skybox;

void main()
{
    vec3 current_color = vec3(0.0, 0.0, 0.0);

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

    // Useful vectors
    vec3 N = fragment.normal;
    float eta = 0.751f;
    float attenuation_distance = 1.0f;
    if (gl_FrontFacing == false) {
		N = -N;
        eta = 1.0f / eta;
        attenuation_distance = length(fragment.position - camera_position);

	} else {
        // float water_height = 7.0f;
        // attenuation_distance = water_height / abs(fragment.position.z);
        attenuation_distance = 0;
    }

    // Refract vector
    vec3 I = normalize(fragment.position - camera_position);
    vec3 texture_coords = refract(I, N, eta);
    if (texture_coords.x == 0 && texture_coords.y == 0 && texture_coords.z == 0)
        texture_coords = reflect(I, N);

    vec3 mapped_color = vec3(texture(image_skybox, texture_coords));
    current_color = mapped_color;

    // Color attenuation
    vec3 water_color = vec3(0.016,0.659,0.878); // TODO ?
    float alpha = 0.12f; // Water attenuation coefficient at 350nm
    float scale = .05f; // Scale correction coefficient
    float attenuation = exp(-alpha * scale * attenuation_distance);
    current_color = current_color * attenuation + (1 - attenuation) * water_color;
   
    // Specular sunlight
    // TODO
    // float specular_magnitude = pow(max(dot(R, Cn), 0.0), specularExp) * specular;
    // current_color += specular_magnitude * light_color;

    FragColor = vec4(current_color, 1.0); // Note: the last alpha component is not used here
}
