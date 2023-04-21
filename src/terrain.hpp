#pragma once

#include "cgp/cgp.hpp"
#include "environment.hpp"

using namespace cgp;

struct perlin_noise_parameters
{
	float persistency;
	float frequency_gain ;
	int octave;

	perlin_noise_parameters(float persistency_, float frequency_gain_, float octave_) {
		persistency = persistency_;
		frequency_gain = frequency_gain_;
		octave = octave_;
	}
};

struct terrain
{
	cgp::mesh generate_terrain_mesh();

	vec3 getPoint(int i, int j, int k);

	bool is_in_voxel(cgp::vec3& pos);

	vec3 getCenterPoint(int edge_idx);

	float evaluate_noise_level(float x, float y, float z);
};

