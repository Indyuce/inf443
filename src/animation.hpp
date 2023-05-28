#include "cgp/cgp.hpp"
#include "terrain.hpp"
#include "implicit_surface/field_function.hpp"
#pragma once


struct fish {
	cgp::vec3 position;
	cgp::vec3 direction;
	float speed;
	cgp::mesh_drawable model;
	int modelId;
	float frequency;

};

struct alga {
	cgp::vec3 position;
	float amplitude;
	float frequency;
	float rotation;
};

struct alga_group {
	std::vector<alga> algas;
};


struct fish_manager {
	fish_manager();
	void refresh(field_function_structure field,float t);
	std::vector<fish> fishes;
	std::vector<alga_group> alga_groups;
	int num_group,min_alga_per_group,max_alga_per_group;
	float separation_coef, alignement_coef, cohesion_coef,fish_radius,fish_speed,obstacle_radius,obstacle_coef;
	cgp::vec3 calculate_separation(int i);
	cgp::vec3 calculate_alignement(int i);
	cgp::vec3 calculate_cohesion(int i);
	cgp::vec3 calculate_out_of_bound_force(int i);
};
static const int XY_LENGTH = 100;
static const int Z_LENGTH = 100;
static int counter;