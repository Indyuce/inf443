#pragma once

#include "cgp/cgp.hpp"

using namespace cgp;



struct environment_structure : environment_generic_structure
{


	float offset = -9.0f;
	float hmul = 1.0f;
		float mult = 7.0f;

	// Light parameters
	vec3 light_color = { .988f, .898f, .439f };
	vec2 light_direction = { 0.0f, 90.0f };
	float ambiant = 0.3f;
	float diffuse = 0.8f;
	float specular = 0.03f;
	float direct = 2.35f;
	int directExp = 410;
	int specularExp = 2;
	float fog_distance = 1000.0f;
	float attenuation_distance = 70.0f;

	// Color of the background of the scene
	vec3 background_color = { 1,1,1 }; // Used in the main program

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
