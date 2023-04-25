#include "cgp/cgp.hpp"

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


class fish_manager {
	public:
		fish_manager(float separation_coef, float cohesion_coef, float alignement_coef,float fish_radius,float fish_speed, int num_alga_group, int min_alga, int max_alga);
		void add(fish fish);
		void refresh();
		std::vector<fish> fishes;
		std::vector<alga_group> alga_groups;
	private:
		float separation_coef, alignement_coef, cohesion_coef,fish_radius,fish_speed;
		cgp::vec3 calculate_separation(int i);
		cgp::vec3 calculate_alignement(int i);
		cgp::vec3 calculate_cohesion(int i);
};
