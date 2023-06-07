#pragma once

#include "cgp/cgp.hpp"
#include "environment.hpp"
#include "living_entities.hpp"
#include "terrain.hpp"
#include "water_surface.hpp"
#include "particles.hpp"
#include "camera_movement.hpp"
#include "implicit_surface/implicit_surface.hpp"
#include "multipass/multipass_structure.hpp"
#include <random>

// This definitions allow to use the structures: mesh, mesh_drawable, etc. without mentionning explicitly cgp::
using cgp::mesh;
using cgp::mesh_drawable;
using cgp::vec3;
using cgp::numarray;
using cgp::timer_basic;

// The structure of the custom scene
struct scene_structure : cgp::scene_inputs_generic {

	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	camera_controller_orbit_euler camera_control;
	camera_projection_perspective camera_projection;
	camera_movement_structure camera_movement;
	window_structure window;
	multipass_structure multipass_rendering;

	environment_structure environment;   // Standard environment controler
	input_devices inputs;                // Storage for inputs status (mouse, keyboard, window dimension)
	timer_basic timer;                   // For timer

	// Random
	std::mt19937 rand_gen;
	std::uniform_real_distribution<> rand_double;

	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //

	// Skybox
	cgp::skybox_drawable skybox;
	cgp::skybox_drawable underwater_skybox;

	// Terrain
	implicit_surface_structure implicit_surface; // Structures used for the implicit surface (*)
	field_function_structure field_function;     // A Parametric function used to generate the discrete field (*)
	water_surface_structure water_surface;        // Mesh for water surface

	// Fishes
	fish_manager fish_manager;
	terrain_structure terrain;

	// Particles
	particle_manager particles;

	// ****************************** //
	// Functions
	// ****************************** //

	void initialize(); // Standard initialization to be called before the animation loop
	void display_frame();
	void display_scene();
	// The frame display to be called within the animation loop
	void display_semi_transparent(vec3 const& camera_position); // Display semi transparent tiles
	void display_gui(); // The display of the GUI, also called within the animation loop

	float get_height(float x, float y); //Computes the height at a given x and y.
	vec3 random_vector();
	float random_offset();
	void symetrize_camera_view();

	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();
};