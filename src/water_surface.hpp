#pragma once

#include "cgp/cgp.hpp"

using namespace cgp;


// TODO could be done with model hierarchy ??
struct water_surface_structure
{

	/// <summary>
	/// Surface is subdivised into chunks. Since water height is computed dynamically at every
	/// frame, without shaders, the surface is just a MERE plane.
	/// 
	/// The X are rendered using a complex vertex shaders and can have Gerstner waves.
	/// Other surface chunks are just planes which only call a fragment shader for optimisation.
	/// They also have a fog shader which blends into the horizon.
	/// 
	/// Scheme:
	/// |-----------|
	/// |           |
	/// |---XXXXX---|
	/// |   XXXXX   |
	/// |---XXXXX---|
	/// |           |
	/// |-----------|
	/// 
	/// </summary>
	mesh_drawable center, positive_x, negative_x, positive_y, negative_y;
	float center_length, expanded_tiles_displacement, expanded, total_length;

	void initialize_models();

	void update_positions_and_draw(vec3 const& camera_position, environment_generic_structure& environment);

	void set_shaders(opengl_shader_structure& shader);

	void set_textures(opengl_texture_image_structure& sand, opengl_texture_image_structure& skybox, opengl_texture_image_structure& texture_scene);
};

