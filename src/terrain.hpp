#pragma once

#include "cgp/cgp.hpp"
#include "environment.hpp"
#include "chunk_data.hpp"

using namespace cgp;

struct perlin_noise_parameters
{
	float persistency;
	float frequency_gain;
	int octave;
	float scale;
	float multiplier;

	perlin_noise_parameters(float persistency_, float frequency_gain_, int octave_, float scale_, float multiplier_) {
		persistency = persistency_;
		frequency_gain = frequency_gain_;
		octave = octave_;
		scale = scale_;
		multiplier = multiplier_;
	}
};

struct terrain
{
	// std::unordered_map<int, chunk_data> loaded_chunks;

	chunk_data* generate_chunk_data(int chunk_x, int chunk_y);

	vec3 interpolate(vec3& ref_point, int edge_idx);

	float potential(vec3& pos);

	float potential(float x, float y, float z);

	bool is_in_voxel(float x, float y, float z);
};

