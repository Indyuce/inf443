#pragma once

#include "water_surface.hpp"
#include "environment.hpp"

mesh generate_grid(float const& x, float const& y, float const& resolution, bool const& vertices) {
	return mesh_primitive_grid(
		{ -x / 2.0f, -y / 2.0f, 0.0f },
		{ x / 2.0f, -y / 2.0f, 0.0f },
		{ x / 2.0f, y / 2.0f, 0.0f },
		{ -x / 2.0f, y / 2.0f, 0.0f },
		vertices ? (int) x * resolution : 2,
		vertices ? (int) y * resolution : 2);
}

void water_surface_structure::initialize_models() {

	// Edit mesh parameters here
	/************************************************************/
	center_length             = 600.0f;
	float const mult          = 10.0f; // Expanded water surface is 3x bigger

	// Update variables
	expanded                    = center_length * (mult - 1.0) / 2.0f;
	expanded_tiles_displacement = (center_length + expanded) / 2.0f;
	total_length                = center_length + expanded;

	// Generate meshes
	center.initialize_data_on_gpu(generate_grid(center_length, center_length, .5f, true));
	positive_x.initialize_data_on_gpu(generate_grid(expanded, center_length * mult, -1, false));
	negative_x.initialize_data_on_gpu(generate_grid(expanded, center_length * mult, -1, false));
	positive_y.initialize_data_on_gpu(generate_grid(center_length, expanded, -1, false));
	negative_y.initialize_data_on_gpu(generate_grid(center_length, expanded, -1, false));
}

void water_surface_structure::update_positions_and_draw(vec3 const& camera_position, environment_generic_structure& environment) {

	// Update pos
	vec3 const player_displacement = vec3(camera_position.x, camera_position.y, 0);

	center.model.translation = player_displacement;
	positive_x.model.translation = player_displacement + vec3(expanded_tiles_displacement, 0.0f, 0.0f);
	negative_x.model.translation = player_displacement + vec3(-expanded_tiles_displacement, 0.0f, 0.0f);
	positive_y.model.translation = player_displacement + vec3(0.0f, expanded_tiles_displacement, 0.0f);
	negative_y.model.translation = player_displacement + vec3(0.0f, -expanded_tiles_displacement, 0.0f);

	// Draw
	cgp::draw(center, environment);
	cgp::draw(positive_x, environment);
	cgp::draw(negative_x, environment);
	cgp::draw(positive_y, environment);
	cgp::draw(negative_y, environment);
}

void water_surface_structure::set_shaders(opengl_shader_structure& shader) {
	center.shader = shader;
	positive_x.shader = shader;
	negative_x.shader = shader;
	positive_y.shader = shader;
	negative_y.shader = shader;
}

void set_textures_drawable(mesh_drawable& drawable, opengl_texture_image_structure& texture_sand, opengl_texture_image_structure& texture_skybox, opengl_texture_image_structure& texture_scene, opengl_texture_image_structure& texture_extra) {
	drawable.supplementary_texture["texture_sand"] = texture_sand;
	drawable.supplementary_texture["texture_scene"] = texture_scene;
	drawable.supplementary_texture["texture_skybox"] = texture_skybox;
	drawable.supplementary_texture["texture_extra"] = texture_extra;
}

void water_surface_structure::set_textures(opengl_texture_image_structure& texture_sand, opengl_texture_image_structure& texture_skybox, opengl_texture_image_structure& texture_scene, opengl_texture_image_structure& texture_extra) {
	set_textures_drawable(center, texture_sand, texture_skybox, texture_scene, texture_extra);
	set_textures_drawable(positive_x, texture_sand, texture_skybox, texture_scene, texture_extra);
	set_textures_drawable(negative_x, texture_sand, texture_skybox, texture_scene, texture_extra);
	set_textures_drawable(positive_y, texture_sand, texture_skybox, texture_scene, texture_extra);
	set_textures_drawable(negative_y, texture_sand, texture_skybox, texture_scene, texture_extra);
}
