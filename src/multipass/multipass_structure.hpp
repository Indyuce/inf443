#pragma once

#include "cgp/cgp.hpp"
#include "custom_fbo_structure.hpp"

struct multipass_structure {
	custom_fbo_structure fbo_pass_1;
	cgp::opengl_fbo_structure fbo_pass_2;

	cgp::mesh_drawable quad_pass_2;
	cgp::mesh_drawable quad_pass_3;

	void initialize(std::string project_path);
	void clear_screen();
	void update_screen_size(int width, int height);

	void start_pass_1();
	void end_pass_1();

	void start_pass_2();
	void draw_pass_2(cgp::environment_generic_structure const& environment);
	void end_pass_2();

	void start_pass_3();
	void draw_pass_3(cgp::environment_generic_structure const& environment);
	void end_pass_3();
};
