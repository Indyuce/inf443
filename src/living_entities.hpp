#pragma once

#include "cgp/cgp.hpp"
#include "implicit_surface/field_function.hpp"

struct fish
{
	cgp::vec3 position;
	cgp::vec3 direction;
	float speed;
	cgp::mesh_drawable model;
	int modelId;
	float frequency;

	float last_particle_emission;
	float last_particle_emission_test;
};

struct fish_manager
{
	fish_manager();

	float domain_x, domain_y, domain_z;
	bool grid_filled;
	int ticks, grid_step;

	std::vector<fish> fishes;
	std::vector<cgp::mesh_drawable> fish_models;
	std::vector<fish> ***fish_grid;
	int fish_groups_number;
	int fishes_per_group;
	float separation_coef, alignement_coef, cohesion_coef, fish_radius, fish_speed, obstacle_radius, obstacle_coef;

	void initialize(cgp::vec3 domain, float floor_level, std::string project_path);

	void refresh(field_function_structure field, float t);

	void refresh_grid();

	std::vector<fish> get_neighboring_fishes(fish fish);

	cgp::vec3 calculate_separation(fish fish);

	cgp::vec3 calculate_alignement(fish fish);

	cgp::vec3 calculate_cohesion(fish fish);

	cgp::vec3 calculate_out_of_bound_force(fish fish);
};