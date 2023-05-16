#pragma once


#include "cgp/cgp.hpp"
#include "environment.hpp"
#include "animation.hpp"
#include "terrain.hpp"

#include "implicit_surface/implicit_surface.hpp"

// This definitions allow to use the structures: mesh, mesh_drawable, etc. without mentionning explicitly cgp::
using cgp::mesh;
using cgp::mesh_drawable;
using cgp::vec3;
using cgp::numarray;
using cgp::timer_basic;

// The structure of the custom scene
struct scene_structure : cgp::scene_inputs_generic {

	scene_structure();
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //
	camera_controller_orbit_euler camera_control;
	camera_projection_perspective camera_projection;
	window_structure window;

	environment_structure environment;   // Standard environment controler
	input_devices inputs;                // Storage for inputs status (mouse, keyboard, window dimension)
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	// marching cube
	implicit_surface_structure implicit_surface; // Structures used for the implicit surface (*)
	field_function_structure field_function;     // A Parametric function used to generate the discrete field (*)


	timer_basic timer;

	/// <summary>
	/// Sphere displayed all around the camera to make sure shaders are applied
	/// in all directions. This is particularily important for direct light source
	/// illumination shaders.
	/// </summary>
	mesh_drawable camera_sphere;

	// Terrain
	terrain terrain_gen;
	chunk_data* drawable_chunk;

	/**
	 *  Fishes
	*/
	int num_fishes;
	float dt;
	float t;
	
	cgp::mesh_drawable fish0;
	cgp::mesh_drawable fish1;
	cgp::mesh_drawable fish2;
	cgp::mesh_drawable fish3;
	cgp::mesh_drawable fish4;
	cgp::mesh_drawable jellyfish;
	cgp::mesh_drawable alga;

	fish_manager fish_manager;

	// ****************************** //
	// Functions
	// ****************************** //

	void initialize();

	vec3 get_camera_position();

	// Standard initialization to be called before the animation loop
	void display_frame(); // The frame display to be called within the animation loop
	void display_gui();   // The display of the GUI, also called within the animation loop
	void initialize_models(); // Initialize the models of the fishes.


	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();
	
};