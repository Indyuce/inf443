#pragma once

#include "cgp/cgp.hpp"

using namespace cgp;



struct environment_structure : environment_generic_structure
{
	struct { // Elements of the domain
		int size_coef = 1;
		int samples = 100 * size_coef;
		cgp::vec3 length = { 200 * size_coef, 200 * size_coef, 100 };
	} domain;

	opengl_shader_structure shader;

	// Isovalue used during the marching cube
	float isovalue = 0.4f;

	// Light parameters
	vec3 player_light = { 1.0f, 1.0f, 1.0f };
	vec3 light_color = { .988f, .898f, .439f };
	vec2 light_direction = { 120.0f, -95.0f }; // Matches the skybox

	// Phong illumination
	float ambiant = 0.3f;
	float diffuse = 0.8f;
	float specular = 0.03f;
	int specular_exp = 2;

	// Direct illumination
	float direct = 1.6f;
	int direct_exp = 800;

	// Player Flashlight
	float flashlight = 2.41f;
	int flashlight_exp = 33;
	float flashlight_dist = 10.0f;

	// Distance-based fog
	float fog_distance = 150.0f;
	// float attenuation_distance = 100.0f;

	// Scene background color
	vec3 background_color = { 0, 67.0f / 255.0f, 226.0f / 255.0f }; // Used in the main program

	// The position/orientation of a camera that can rotates freely around a specific position
	mat4 camera_view;

	// A projection structure (perspective or orthogonal projection)
	mat4 camera_projection;

	// Additional uniforms that can be attached to the environment if needed (empty by default)
	uniform_generic_structure uniform_generic;
	
	void send_opengl_uniform(opengl_shader_structure const& shader, bool expected = true) const override;
};



// Global variables about your project
struct project {

	// Global variable storing the relative path to the root of the project (access to shaders/, assets/, etc)
	static std::string path;

	// ImGui Scale: change this value (default=1) for larger/smaller gui
	static float gui_scale;

};
