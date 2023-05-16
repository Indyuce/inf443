#pragma once

#include "cgp/cgp.hpp"
#include "field_function.hpp"

// Element of the GUI that are not already stored in other structures
struct gui_parameters {

	struct { // Elements of the domain
		int samples = 10;
		cgp::vec3 length = { 200, 200, 50 };
	} domain;

	// Isovalue used during the marching cube
	float isovalue = 0.0f;
};


void display_gui_implicit_surface(bool& is_update_field, bool& is_update_marching_cube, bool& is_save_obj, gui_parameters& gui, field_function_structure& field_function);