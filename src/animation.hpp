#include "cgp/cgp.hpp"
#include "implicit_surface/field_function.hpp"
#pragma once

struct fish
{
	cgp::vec3 position;
	cgp::vec3 direction;
	float speed;
	cgp::mesh_drawable model;
	int modelId;
	float frequency;
};

struct alga
{
	cgp::vec3 position;
	float amplitude;
	float frequency;
	float rotation;
};

struct alga_group
{
	std::vector<alga> algas;
};

struct fish_manager
{
	float domain_x, domain_y, domain_z;

	fish_manager();

	void initialize(cgp::vec3 domain, std::string project_path);

	// Fishes
	int ticks;
	std::vector<fish> fishes;
	std::vector<cgp::mesh_drawable> fish_models;
	cgp::mesh_drawable alga_model;
	std::vector<fish> ***fish_grid;
	int fish_groups_number;
	int fishes_per_group;
	float separation_coef, alignement_coef, cohesion_coef, fish_radius, fish_speed, obstacle_radius, obstacle_coef;

	// Alga
	std::vector<alga_group> alga_groups;
	int num_group, min_alga_per_group, max_alga_per_group, grid_step;

	void refresh(field_function_structure field, float t);

	void refresh_grid();

	std::vector<fish> get_neighboring_fishes(fish fish);

	cgp::vec3 calculate_separation(fish fish);

	cgp::vec3 calculate_alignement(fish fish);

	cgp::vec3 calculate_cohesion(fish fish);

	cgp::vec3 calculate_out_of_bound_force(fish fish);
};