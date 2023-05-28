#pragma once

#include "cgp/cgp.hpp"
#include "implicit_surface/field_function.hpp"

using namespace cgp;

struct environment_structure : environment_generic_structure {

	// Water
	float water_level = 0.0f;
	float floor_level = -100.0f;
	vec3 fog_color = { 0.016f, 0.659f, 0.878f }; // Used in the main program
	vec3& background_color = fog_color; // Used in the main program
	bool surf_height; // Boolean to enable height-based water shading

	// Light parameters
	vec3 player_light = { 1.0f, 1.0f, 1.0f };
	vec3 light_color = { .988f, .898f, .439f };
	vec2 light_direction = { 120.0f, -95.0f }; // Matches the skybox
	float water_attenuation_coefficient = 0;// .12f;

	// Phong illumination
	float ambiant = 0.3f;
	float diffuse = 0.8f;
	float specular = 0.03f;
	int specular_exp = 2;

	// Direct illumination
	float direct = 1.6f;
	int direct_exp = 800;

	// Player Flashlight
	float flashlight = 0;//2.41f;
	int flashlight_exp = 33;
	float flashlight_dist = 10.0f;

	// Terrain
	opengl_shader_structure shader;
	float isovalue = 0.4f; // Isovalue used during the marching cube

	// Domain and physics
	struct { 
		int resolution = 2;
		cgp::vec3 length = { 200, 200, 50 };
	} domain;
	float scale = 0.05f;

	mat4 camera_view; // The position/orientation of a camera that can rotates freely around a specific position
	mat4 camera_projection; // A projection structure (perspective or orthogonal projection)

	// Additional uniforms that can be attached to the environment if needed (empty by default)
	uniform_generic_structure uniform_generic;

	vec3 get_camera_position() const;
	
	void send_opengl_uniform(opengl_shader_structure const& shader, bool expected = true) const override;

	float get_water_level(vec3 const& position, float& time) const;
};

struct gerstner_wave {
	vec2 direction;
	float amplitude;
	float steepness;
	float frequency;
	float speed;

	gerstner_wave::gerstner_wave() {
	}

	gerstner_wave::gerstner_wave(vec2 direction_, float amplitude_, float steepness_, float frequency_, float speed_) {
		direction = direction_;
		amplitude = amplitude_;
		steepness = steepness_;
		frequency = frequency_;
		speed = speed_;
	}
};

// Global variables about your project
struct project {

	// Global variable storing the relative path to the root of the project (access to shaders/, assets/, etc)
	static std::string path;

	// ImGui Scale: change this value (default=1) for larger/smaller gui
	static float gui_scale;
};