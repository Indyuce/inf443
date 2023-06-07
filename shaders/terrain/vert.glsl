#version 330 core

// Vertex shader - this code is executed for every vertex of the shape

// Inputs coming from VBOs
layout (location = 0) in vec3 vertex_position; // vertex position in local space (x,y,z)
layout (location = 1) in vec3 vertex_normal;   // vertex normal in local space   (nx,ny,nz)
layout (location = 2) in vec3 vertex_color;    // vertex color      (r,g,b)
layout (location = 3) in vec2 vertex_uv;       // vertex uv-texture (u,v)

// Output variables sent to the fragment shader
out struct fragment_data
{
    vec3 position; // vertex position in world space
    vec3 normal;   // normal position in world space
    vec3 color;    // vertex color
    vec2 uv;       // vertex uv
} fragment;

// Uniform variables expected to receive from the C++ program
uniform mat4 model; // Model affine transform matrix associated to the current shape
uniform mat4 view;  // View matrix (rigid transform) of the camera
uniform mat4 projection; // Projection (perspective or orthogonal) matrix of the camera

uniform mat4 modelNormal; // Model without scaling used for the normal. modelNormal = transpose(inverse(model))

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
	bool is_sand;           // If sand, sand can turn into stone if gardient is not vertical
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

uniform sampler2D height_map;
uniform sampler2D rock_height;

uniform float ridge_coefficient;
uniform float sand_texture_scale;
uniform float time;

void main()
{

	// The position of the vertex in the world space
	vec4 position = model * vec4(vertex_position, 1.0);

	// The normal of the vertex in the world space
	vec4 normal = modelNormal * vec4(vertex_normal, 0.0);

	// Height map
	vec2 fixed_vertex_uv = vertex_position.xy * sand_texture_scale;
    vec3 height_map      = (texture(height_map, fixed_vertex_uv).xyz * 2.0f) - 1.0f; // Unpack
    
    // Sand-specific
    if (material.texture_settings.is_sand) {
        float rockiness = 1.0f - pow(max(0.0f, dot(normal.xyz, vec3(0, 0, 1))), 3.0f);
        height_map = mix(height_map, (texture(rock_height, fixed_vertex_uv).xyz * 2.0f) - 1.0f, rockiness);
    }

	position += normalize(normal) * vec4(height_map, 0) * ridge_coefficient;

	// The projected position of the vertex in the normalized device coordinates:
	vec4 position_projected = projection * view * position;

	// Fill the parameters sent to the fragment shader
	fragment.position = position.xyz;
	fragment.normal   = normal.xyz;
	fragment.color    = vertex_color;
	fragment.uv       = fixed_vertex_uv;

	// gl_Position is a built-in variable which is the expected output of the vertex shader
	gl_Position = position_projected; // gl_Position is the projected vertex position (in normalized device coordinates)
}
