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

// This blends two normals by taking the second one and applying to
// it the rotation corresponding to the first normal.
// Ref https://www.shadertoy.com/view/4t2SzR
vec3 blend_normals(vec3 n1, vec3 n2)
{
    // Unpack
	n1 = n1*2.0 - 1.0;
    n2 = n2*2.0 - 1.0;
    
    mat3 nBasis = mat3(vec3(n1.z, n1.y, -n1.x), // +90 degree rotation around y axis
        			   vec3(n1.x, n1.z, -n1.y), // -90 degree rotation around x axis
        			   vec3(n1.x, n1.y,  n1.z));
	
    return normalize(n2.x*nBasis[0] + n2.y*nBasis[1] + n2.z*nBasis[2]);
}

void main()
{
    vec3 current_color = vec3(0.0, 0.0, 0.0);

    // Texture
    /************************************************************/
	vec2 uv_image       = fragment.uv; 
	vec3 texture_color  = texture(image_texture, uv_image).xyz;
    vec3 fragment_color = fragment.color * texture_color;

    // Normal map
    /************************************************************/
    vec3 fragment_normal = normalize(fragment.normal);
    vec3 map_normal      = texture(normal_map, uv_image).xyz * 2 - 1;
    //vec3 N               = blend_normals(fragment_normal, map_normal);
    vec3 N = map_normal;

    // Useful vectors
    /************************************************************/
    vec3 R = reflect(light_direction, N);
    vec3 C = camera_position - fragment.position;
    vec3 Cn = normalize(C);
    
    // Inverse the normal if it is viewed from its back (two-sided surface)
	//  (note: gl_FrontFacing doesn't work on Mac)
    // TODO support material.texture_settings.two_sided
	//if (gl_FrontFacing == false) {
	//	N = -N;
	//}

    // Color Attenuation TODO?
    /************************************************************/
    //float dl = length(camera_position - fragment.position);
    //float attenuation_coef = 1 - min(dl / attenuation_distance, 1);
    vec3 eff_light_color = light_color; //attenuation_coef * light_color;

    // Diffuse sunlight
    float diffuse_magnitude = max(dot(N, -light_direction), 0.0) * diffuse;

    // Specular sunlight
    float specular_magnitude = pow(max(dot(R, Cn), 0.0), specular_exp) * specular;
    
    // Flashlight. Diffuse and specular are the same, because the light source is also the observer
    // Both effects are wrapped up inside of the 'flashlight' coefficient
    float flashlight_magnitude = 0;
    if (flashlight_on)
        flashlight_magnitude = flashlight * pow(max(dot(Cn, camera_direction), 0), flashlight_exp) * max(0, dot(N, Cn));

    // Calculate color
    current_color += ((ambiant + diffuse_magnitude) * fragment_color + specular_magnitude) * eff_light_color + flashlight_magnitude * material.color;
    
    // Water attenuation
    float attenuation_distance = length(C);
    float attenuation = exp(-water_attenuation_coefficient * scale * attenuation_distance);
    current_color = current_color * attenuation + (1 - attenuation) * fog_color;
    
    FragColor = vec4(current_color, 1.0); // Note: the last alpha component is not used here
}
