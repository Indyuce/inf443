#pragma once

#include "cgp/cgp.hpp"
#include "custom_fbo_structure.hpp"

struct multipass_structure {
	custom_fbo_structure fbo_pass_1;

	cgp::mesh_drawable quad_pass_2;

	void initialize(std::string project_path);
	void set_shader_pass_2(cgp::opengl_shader_structure const& shader);
	void update_screen_size(int width, int height);
	void clear_screen(cgp::vec3 const background_color = { 1,1,1 }) const;


	void start_pass_1(cgp::vec3 const background_color = { 1,1,1 });
	void end_pass_1();

	void start_pass_2(cgp::vec3 const background_color = { 1,1,1 });
	void draw_pass_2(cgp::environment_generic_structure const& environment);
	void end_pass_2();
};
