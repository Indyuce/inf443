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

uniform samplerCube image_skybox;

void main()
{
    vec3 current_color = vec3(0.0, 0.0, 0.0);

    // Gradient cursor
    float p = .5 + fragment.position.z / 10.0f;

    vec3 color1 = vec3(0.012,0.478,0.871);
    vec3 color2 = vec3(0.012,0.898,0.718);
    current_color = p * color2 + (1 - p) * color1;
    
    // Useful vectors
    vec3 N = normalize(fragment.normal);

    //if (gl_FrontFacing == false) {
	//	N = -N;
	//}

    // Refract vector
    vec3 texture_coords = refract(normalize(fragment.position - camera_position), -N, 1.332f);
    vec3 mapped_color = vec3(texture(image_skybox, texture_coords));
   
    // Specular sunlight
    //float specular_magnitude = pow(max(dot(R, Cn), 0.0), specularExp) * specular;
    //current_color += specular_magnitude * light_color;

    FragColor = vec4(mapped_color, 0.7); // Note: the last alpha component is not used here
}
