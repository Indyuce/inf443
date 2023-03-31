#pragma once


#include "cgp/cgp.hpp"
#include "environment.hpp"
#include "terrain.hpp"


// This definitions allow to use the structures: mesh, mesh_drawable, etc. without mentionning explicitly cgp::
using cgp::mesh;
using cgp::mesh_drawable;
using cgp::vec3;
using cgp::numarray;
using cgp::timer_basic;

// Variables associated to the GUI
struct gui_parameters {
	bool display_frame = true;
	bool display_wireframe = false;

	bool depth_buffer;
};

struct light_parameters {
	vec3 light_color = { 1,1,1 };
	vec3 light_position = { -2, 2, 2 };

	float ambiant = 0.3f;
	float diffuse = 0.8f;
	float specular = 0.03f;
	float direct = 0.03f;

	int directExp = 2;
	int specularExp = 2;
	float fog_distance = 50.0f;
	float attenuation_distance = 70.0f;
};

// The structure of the custom scene
struct scene_structure : cgp::scene_inputs_generic {

	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //
	camera_controller_orbit_euler camera_control;
	camera_projection_perspective camera_projection;
	window_structure window;

	mesh_drawable global_frame;          // The standard global frame
	environment_structure environment;   // Standard environment controler
	input_devices inputs;                // Storage for inputs status (mouse, keyboard, window dimension)
	gui_parameters gui;                  // Standard GUI element storage
	timer_basic timer;
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	cgp::mesh terrain_mesh;
	cgp::mesh_drawable terrain;
	cgp::mesh_drawable tree;
	cgp::mesh_drawable camera_sphere;
	cgp::mesh_drawable grass;
	cgp::mesh_drawable sphere_light;

	std::vector<cgp::vec3> treePositions;
	perlin_noise_parameters parameters;
	light_parameters light_parameters;

	// ****************************** //
	// Functions
	// ****************************** //

	void initialize();    // Standard initialization to be called before the animation loop
	void display_frame(); // The frame display to be called within the animation loop
	void display_gui();   // The display of the GUI, also called within the animation loop

	vec3 get_camera_position();
	float calculate_light_z_buffer();

	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();
};





