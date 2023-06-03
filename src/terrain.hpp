#pragma once

#include "cgp/cgp.hpp"

struct alga
{
	cgp::vec3 position;
	float amplitude;
	float frequency;
	float rotation;
	float scale;
};

struct alga_group
{
	std::vector<alga> algas;
};

// Handles non living objects like rocks and algas
struct terrain_structure {

	// Alga
	static float const DEFAULT_ALGA_SCALE;
	cgp::mesh_drawable alga_model;
	std::vector<alga_group> alga_groups;
	int num_group, min_alga_per_group, max_alga_per_group;

	void initialize(std::string project_path);
};