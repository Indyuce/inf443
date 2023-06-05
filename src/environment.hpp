#pragma once

#include "cgp/cgp.hpp"
#include "implicit_surface/field_function.hpp"

using namespace cgp;

struct environment_structure : environment_generic_structure {

	// Water
	float water_level = 0.0f;
	float floor_level = -200.0f;
	vec3 fog_color = { 0.016f, 0.659f, 0.878f };
	bool surf_height; // Boolean to enable height-based water shading
	float water_optical_index = 1.332f; // Optical ratio of water
	float sand_texture_scale = 0.01f;

	// Light parameters
	vec3 player_light = { 1.0f, 1.0f, 1.0f };
	vec3 light_color = { 1,1,1 };//{ .988f, .898f, .439f };
	vec2 light_direction = { -115.7f, 156.9f }; // Matches the skybox
	float water_attenuation_coefficient = .12f;
	float water_reflection_coefficient = .5f; // How much water will reflect when standing above surface
	float fog_distance = 2900.0f;
	float bloom_threshold = .7f;

	// Direct illumination
	float direct = 1.6f;
	int direct_exp = 800;

	// Player Flashlight
	bool flashlight_on = false;
	float flashlight = 2.41f;
	int flashlight_exp = 33;
	float flashlight_dist = 10.0f;

	// Terrain
	opengl_shader_structure shader;
	float isovalue = 0.4f; // Isovalue used during the marching cube
	float terrain_ridges = 2.0f;

	// Domain and physics
	struct { 
		int resolution = 1;
		cgp::vec3 length = { 1000, 1000, 50 };
	} domain;
	float scale = 0.02f;

	mat4 camera_view; // The position/orientation of a camera that can rotates freely around a specific position
	mat4 camera_projection; // A projection structure (perspective or orthogonal projection)

	// Additional uniforms that can be attached to the environment if needed (empty by default)
	uniform_generic_structure uniform_generic;

	vec3 get_camera_position() const;
	
	void send_opengl_uniform(opengl_shader_structure const& shader, bool expected = true) const override;
};

// Global variables about your project
struct project {

	// Global variable storing the relative path to the root of the project (access to shaders/, assets/, etc)
	static std::string path;

	// ImGui Scale: change this value (default=1) for larger/smaller gui
	static float gui_scale;
};