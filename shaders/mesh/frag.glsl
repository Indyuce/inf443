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


// Uniform values that must be send from the C++ code
// ***************************************************** //

uniform sampler2D image_texture;   // Texture image identifiant

uniform mat4 view;       // View matrix (rigid transform) of the camera - to compute the camera position


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

uniform float ambiant;
uniform float diffuse;
uniform float specular;
uniform int specularExp;
uniform float direct;
uniform int directExp;

uniform vec3 light_color;
uniform vec3 light_position;
uniform float light_z_buffer;

uniform vec3 fog_color;
uniform float fog_distance;
uniform float attenuation_distance;

uniform bool depth_buffer;

float LinearizeDepth()
{
    float zNear = 0.5;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 2000.0; // TODO: Replace by the zFar  of your perspective projection
    float depth = gl_FragCoord.z;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{
	// Visualize depth buffer
	if (depth_buffer) {
		FragColor = vec4(vec3(LinearizeDepth()), 1.0);
		return;
	}
	
	mat3 O = transpose(mat3(view)); // Get the orientation matrix
	vec3 last_col = vec3(view * vec4(0.0, 0.0, 0.0, 1.0)); // Get the last column
	vec3 camera_position = -O * last_col; // Compute the position of the center of the camera

	// Renormalize normal
	vec3 N = normalize(fragment.normal);

	// Inverse the normal if it is viewed from its back (two-sided surface)
	// (note: gl_FrontFacing doesn't work on Mac)
	if (material.texture_settings.two_sided && gl_FrontFacing == false) {
		N = -N;
	}
	
	// =================================================================================================
	// Color Attenuation
	// =================================================================================================
	float dl = length(light_position - fragment.position);
	float attenuation_coef = 1 - min(dl / attenuation_distance, 1);
	vec3 eff_light_color = attenuation_coef * light_color;
	
	// =================================================================================================
	// Phong illumination (diffuse, specular)
	// =================================================================================================

	// Unit direction toward the light
	vec3 L = normalize(light_position -fragment.position);
	vec3 C = camera_position - fragment.position;

	// Diffuse coefficient
	float diffuse_component = max(dot(N,L),0.0);

	// Specular coefficient
	float specular_component = 0.0;
	if(diffuse_component>0.0){
		vec3 R = reflect(-L,N); // reflection of light vector relative to the normal.
		vec3 V = normalize(camera_position - fragment.position);
		specular_component = pow( max(dot(R,V),0.0), specularExp); //material.phong.specular_exponent );
	}
	
	// =================================================================================================
	// Direct illumination
	// =================================================================================================
	float direct_component = 0;
	//if (20 < gl_FragCoord.z) {
		vec3 D = normalize(camera_position - light_position);
		float direct_value = max(dot(normalize(C), D), 0);
		direct_value = pow(direct_value, directExp);
		direct_component = direct * direct_value;
	//}
	
	// =================================================================================================
	// Texture
	// =================================================================================================

	// Current (u, v) coordinates
	vec2 uv_image = vec2(fragment.uv.x, fragment.uv.y);
	if(material.texture_settings.texture_inverse_v) {
		uv_image.y = 1.0-uv_image.y;
	}

	// Get the current texture color
	vec4 color_image_texture = texture(image_texture, uv_image);
	if(material.texture_settings.use_texture == false) {
		color_image_texture = vec4(1.0, 1.0, 1.0, 1.0);
	}
	
	// =================================================================================================
	// Compute Shading
	// =================================================================================================
	vec3 color_object = fragment.color * material.color * color_image_texture.rgb;

	// Compute the final shaded color using Phong model
	float Ka = ambiant; //material.phong.ambient;
	float Kd = diffuse; //material.phong.diffuse;
	float Ks = specular; //material.phong.specular;
	vec3 color_shading = (Ka + Kd * diffuse_component) * color_object + (Ks * specular_component + direct_component) * eff_light_color;
	
	// =================================================================================================
	// Fog
	// =================================================================================================
	float fog_coef = min(length(C) / fog_distance, 1);
	color_shading = (1 - fog_coef) * color_shading + fog_coef * fog_color;

	// Output color, with the alpha component
	FragColor = vec4(color_shading, material.alpha * color_image_texture.a);
}