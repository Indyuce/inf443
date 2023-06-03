#version 330 core

// Fragment shader - this code is executed for every pixel/fragment that belongs to a displayed shape
//
// Compute the color using Phong illumination (ambient, diffuse, specular) 
//  There is 3 possible input colors:
//    - fragment_data.color: the per-vertex color defined in the mesh
//    - material.color: the uniform color (constant for the whole shape)
//    - image_texture: color coming from the texture image
//  The color considered is the product of: fragment_data.color x material.color x image_texture
//  The alpha (/transparent) channel is obtained as the product of: material.alpha x image_texture.a
// 

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

// Coefficients of phong illumination model
struct phong_structure {
	float ambient;      
	float diffuse;
	float specular;
	float specular_exponent;
};

// Settings for texture display
struct texture_settings_structure {
	bool use_texture;       // Switch the use of texture on/off
	bool use_normal_map;    // Switch the use of normal map inside of fragment shader.
	bool texture_inverse_v; // Reverse the texture in the v component (1-v)
	bool two_sided;         // Display a two-sided illuminated surface (doesn't work on Mac)
};

// Material of the mesh (using a Phong model)
struct material_structure
{
	vec3 color;  // Uniform color of the object
	float alpha; // alpha coefficient

	phong_structure phong;                       // Phong coefficients
	texture_settings_structure texture_settings; // Additional settings for the texture
}; 

uniform material_structure material;

uniform bool flashlight_on;
uniform float flashlight;
uniform int flashlight_exp;
uniform vec3 flashlight_color;

uniform vec3 light_color;
uniform vec3 light_direction;

uniform vec3 fog_color;
uniform float scale;
uniform float water_attenuation_coefficient;

uniform sampler2D image_texture;
uniform sampler2D normal_map;
uniform float ridge_coefficient;

// This blends two normals by applying to the second normal
// map the rotation corresponding to the first normal map.
// Ref https://www.shadertoy.com/view/4t2SzR
vec3 NormalBlend_RNM(vec3 n1, vec3 n2)
{
    // Unpack (see article on why it's not just n*2-1)
	n1 = n1 * vec3( 2,  2, 2) + vec3(-1, -1,  0);
    n2 = n2 * vec3(-2, -2, 2) + vec3( 1,  1, -1);
    
    // Blend
    return normalize(n1 * dot(n1, n2) / n1.z - n2);
}

void main()
{
    vec3 current_color = vec3(0.0, 0.0, 0.0);

    // Texture and normal map
    /************************************************************/
    vec3 fragment_color = fragment.color;
    vec3 N = fragment.normal;

    if (material.texture_settings.use_texture) {
	    vec2 uv_image       = fragment.uv;
	    vec3 texture_color  = texture(image_texture, uv_image).xyz;
        fragment_color      = fragment_color * texture_color;

        if (material.texture_settings.use_normal_map) {
            vec3 repacked   = (N + 1.0f) / 2.0f;
            vec3 map_normal = texture(normal_map, uv_image).xyz;
            N               = NormalBlend_RNM(repacked, map_normal);
        }
    }

    // Useful vectors
    /************************************************************/
    vec3 R = reflect(light_direction, N);
    vec3 C = camera_position - fragment.position;
    float attenuation_distance = length(C);
    vec3 Cn = C / attenuation_distance;
    
    // Inverse the normal if it is viewed from its back (two-sided surface)
	// (note: gl_FrontFacing doesn't work on Mac)
    if (material.texture_settings.two_sided && !gl_FrontFacing) {
        N = -N;
    }

    // TODO Color Attenuation
    /************************************************************/
    //float dl = length(camera_position - fragment.position);
    //float attenuation_coef = 1 - min(dl / attenuation_distance, 1);
    vec3 eff_light_color = light_color; //attenuation_coef * light_color;

    // Phong Illumination
    /************************************************************/
    // Diffuse sunlight
    float diffuse_magnitude = max(dot(N, -light_direction), 0.0) * material.phong.diffuse;

    // Specular sunlight
    float specular_magnitude = pow(max(dot(R, Cn), 0.0), material.phong.specular_exponent) * material.phong.specular;
    
    // Flaslight. Handmade
    /************************************************************/
    // Diffuse and specular are the same, because the light source is also the observer
    // Both effects are wrapped up inside of the 'flashlight' coefficient
    float flashlight_magnitude = 0;
    if (flashlight_on)
        flashlight_magnitude = flashlight * pow(max(dot(Cn, camera_direction), 0), flashlight_exp) * max(0, dot(N, Cn));

    // Calculate color
    current_color += ((material.phong.ambient + diffuse_magnitude) * fragment_color + specular_magnitude) * eff_light_color + flashlight_magnitude * material.color;
    
    // Water attenuation
    /************************************************************/
    float attenuation = exp(-water_attenuation_coefficient * scale * attenuation_distance);
    current_color = current_color * attenuation + (1 - attenuation) * fog_color;
    
    // Apply color
    FragColor = vec4(current_color, 1.0); // Note: the last alpha component is not used here
}
