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

// Variables associated to the GUI (buttons, etc)
struct gui_parameters {
	bool display_frame = false;
	bool display_wireframe = false;

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

	environment_structure environment;   // Standard environment controler
	input_devices inputs;                // Storage for inputs status (mouse, keyboard, window dimension)
	gui_parameters gui;                  // Standard GUI element storage
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	timer_basic timer;

	// Terrain
	mesh_drawable terrain_drawable;
	terrain terrain_gen;

	/**
	 *  Fishes
	*/
	float boid_speed;
	float boid_radius;
	float separation_coef;
	float alignement_coef;
	float cohesion_coef;
	float change_color_coef;
	int num_fishes;
	float dt;
	float t;


	cgp::mesh_drawable fish0;
	cgp::mesh_drawable fish1;
	cgp::mesh_drawable fish2;
	cgp::mesh_drawable fish3;
	cgp::mesh_drawable fish4;
	cgp::mesh_drawable jellyfish;
	std::vector<fish> fishes;



	// ****************************** //
	// Functions
	// ****************************** //

	void initialize();

	// Standard initialization to be called before the animation loop
	void display_frame(); // The frame display to be called within the animation loop
	void display_gui();   // The display of the GUI, also called within the animation loop
	void initialize_models(); // Initialize the models of the fishes.
	cgp::vec3 calculate_separation(int i);
	cgp::vec3 calculate_alignement(int i);
	cgp::vec3 calculate_cohesion(int i);

	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();
	

};





