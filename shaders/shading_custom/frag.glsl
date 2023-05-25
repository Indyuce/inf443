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
uniform float direct;
uniform int directExp;

uniform float flashlight;
uniform int flashlight_exp;
uniform float flashlight_dist;
uniform vec3 flashlight_color;

uniform vec3 light_color;
uniform vec3 light_direction;

uniform vec3 fog_color;
uniform float fog_distance;
// uniform float attenuation_distance;

// Direct exposure to sun
float near = 0.1; 
float far  = 1000.0;
float th   = 300.0f;
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{
    vec3 current_color = vec3(0.0, 0.0, 0.0);
  
    // Get camera location
    mat3 O = transpose(mat3(view)); // get the orientation matrix
    vec3 last_col = vec3(view * vec4(0.0, 0.0, 0.0, 1.0)); // get the last column
    vec3 camera_position = -O * last_col;

    // Useful vectors
    vec3 N = normalize(fragment.normal);
    vec3 R = reflect(light_direction, N);
    vec3 C = camera_position - fragment.position;
    vec3 Cn = normalize(C);
    
    // Inverse the normal if it is viewed from its back (two-sided surface)
	//  (note: gl_FrontFacing doesn't work on Mac)
    // TODO support material.texture_settings.two_sided
	//if (gl_FrontFacing == false) {
	//	N = -N;
	//}

    // Color Attenuation TODO once water is implemented
    //float dl = length(camera_position - fragment.position);
    //float attenuation_coef = 1 - min(dl / attenuation_distance, 1);
    vec3 eff_light_color = light_color; //attenuation_coef * light_color;

    // Diffuse sunlight
    float diffuse_magnitude = max(dot(N, -light_direction), 0.0) * diffuse;

    // Specular sunlight
    float specular_magnitude = pow(max(dot(R, Cn), 0.0), specularExp) * specular;
    
    // Direct sunlight
    float depth = LinearizeDepth(gl_FragCoord.z); // divide by far for demonstration
    float direct_magnitude = 0;
    if (depth > th) {
        float direct_value = max(dot(Cn, light_direction), 0); 
        direct_value = pow(direct_value, directExp);
        direct_magnitude = direct * direct_value;
    }
    
    // Flashlight. Diffuse and specular are the same, because the light source is also the observer
    // Both effects are wrapped up inside of the 'flashlight' coefficient
    vec3 V = vec3(vec4(0.0, 0.0, 1.0, 0.0) * view);
    float flashlight_magnitude = flashlight * pow(max(dot(Cn, V), 0), flashlight_exp) * max(0, dot(N, Cn));

    // Calculate color
    current_color += ((ambiant + diffuse_magnitude) * fragment.color + specular_magnitude + direct_magnitude) * eff_light_color + flashlight_magnitude * material.color;
    
    // Fog
    float du = length(C);
    float fog_coef = min(du / fog_distance, 1);
    current_color = (1 - fog_coef) * current_color + fog_coef * fog_color;
    
    FragColor = vec4(current_color, 1.0); // Note: the last alpha component is not used here
}
