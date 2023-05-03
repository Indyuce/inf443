#include "cgp/cgp.hpp"
#include "terrain.hpp"
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
};

struct alga_group {
	std::vector<alga> algas;
};


struct fish_manager {
	fish_manager();
	void add(fish fish);
	void refresh(cgp::grid_3D<float> grid);
	std::vector<fish> fishes;
	std::vector<alga_group> alga_groups;
	float separation_coef, alignement_coef, cohesion_coef,fish_radius,fish_speed,obstacle_radius,obstacle_coef;
	cgp::vec3 calculate_separation(int i);
	cgp::vec3 calculate_alignement(int i);
	cgp::vec3 calculate_cohesion(int i);
	cgp::vec3 calculate_out_of_bound_force(int i);
};
static const int XY_LENGTH = 100;
static const int Z_LENGTH = 70;
static int counter;