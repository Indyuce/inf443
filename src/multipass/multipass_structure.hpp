#pragma once

#include "cgp/cgp.hpp"
#include "custom_fbo_structure.hpp"

struct multipass_structure {
	cgp::opengl_fbo_structure fbo_pass_1; // Displays scene from camera
	custom_fbo_structure fbo_pass_2; // Displays scene from reflection of camera through water surface.
	cgp::opengl_fbo_structure fbo_pass_3; // Displays water surface.

	cgp::mesh_drawable quad_pass_3; // Displays scene behind water surface.
	cgp::mesh_drawable quad_pass_4; // Post processing layer.

	void initialize(std::string project_path);
	void clear_screen();
	void update_screen_size(int width, int height);

	void start_pass_1();
	void end_pass_1();

	void start_pass_2();
	void end_pass_2();

	void start_pass_3();
	void draw_pass_3(cgp::environment_generic_structure const& environment);
	void end_pass_3();

	void start_pass_4();
	void draw_pass_4(cgp::environment_generic_structure const& environment);
	void end_pass_4();
};
