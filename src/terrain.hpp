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

	chunk_data* generate_chunk_data(int chunk_x, int chunk_y, opengl_shader_structure& shader);

	// Compute a grid filled with the value of some scalar function - the size of the grid is given by the domain
	cgp::grid_3D<float> compute_scalar_field(cgp::spatial_domain_grid_3D const& domain);

	float potential(spatial_domain_grid_3D const& domain, vec3 const& pos);

	static const int XY_LENGTH = 100;
	static const int Z_LENGTH = 70;

	// Amount of sampling points for cube marching per unit of length
	static const int PRECISION = 1;

	// Isovalue for marching cubes
	static const float ISOVALUE;

	// Amount of sampling points per chunk horizontally/vertically
	static const int XY_SAMPLES = PRECISION * XY_LENGTH;
	static const int Z_SAMPLES = PRECISION * Z_LENGTH;

	// Inverse of precision: units of length per sampling point
	static const float SCALE;
};

