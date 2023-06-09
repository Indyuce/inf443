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
layout(location=1) out vec4 ExtraColor;
layout(location=2) out vec4 BrightColor;

// Uniform values that must be send from the C++ code
// ***************************************************** //

uniform sampler2D image_texture;   // Texture image identifiant

uniform mat4 view;       // View matrix (rigid transform) of the camera - to compute the camera position

uniform vec3 light; // position of the light


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

uniform vec3 camera_position;
uniform vec3 fog_color;
uniform float water_attenuation_coefficient;
uniform float scale;

// Specific to particles
uniform float opacity_multiplier;

// Depth buffer calculation
/***************************************************************************************************/
uniform float depth_min;
uniform float depth_max;

float get_depth_buffer(float frag_distance) {
	return (frag_distance - depth_min) / depth_max;
}

// Main Shader
/***************************************************************************************************/

void main()
{

	// Texture
	// *************************************** //

	// Current uv coordinates
	vec2 uv_image = vec2(fragment.uv.x, fragment.uv.y);
	if(material.texture_settings.texture_inverse_v) {
		uv_image.y = 1.0-uv_image.y;
	}

	// Get the current texture color
	vec4 color_image_texture = texture(image_texture, uv_image);
	
	// Compute Shading
	// *************************************** //

	// Compute the base color of the object based on: vertex color, uniform color, and texture
	vec3 current_color  = fragment.color * material.color * color_image_texture.rgb;

	// Water attenuation
    /************************************************************/
    float attenuation_distance = length(camera_position - fragment.position);
    float attenuation = exp(-water_attenuation_coefficient * scale * attenuation_distance);
    current_color = current_color * attenuation + (1 - attenuation) * fog_color;
	
	// Texture outputs
    /************************************************************/
	FragColor = vec4(current_color, material.alpha * color_image_texture.a * opacity_multiplier); // Output color, with the alpha component
	ExtraColor = vec4(get_depth_buffer(attenuation_distance), 0.0, 0.0, 0.0);  // Output extra buffers
}