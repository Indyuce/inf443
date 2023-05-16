#pragma once

#include "cgp/cgp.hpp"

struct perlin_noise_params
{
	float persistency;
	float frequency_gain;
	int octave;
	float scale;
	float multiplier;
	float offset;

	perlin_noise_params(float persistency_, float frequency_gain_, int octave_, float scale_, float multiplier_, float offset_);

	perlin_noise_params();
};

// Parametric function defined as a sum of blobs-like primitives
//  f(p) = sa exp(-||p-pa||^2) + sb exp(-||p-pb||^2) + sc exp(-||p-pc||^2) + noise(p)
//   with noise: a Perlin noise
// The operator()(vec3 p) allows to query a value of the function at arbitrary point in space
struct field_function_structure {

	// Query the value of the function at any point p
	float operator()(cgp::vec3 const& p) const;

	/*
	* Perlin noise is added to the total noise level to generate the floor level at z close to 0
	* This noise is multiplied by exp(-z/d) to reduce influence of this noise at higher z values.
	* d is the vertical attenuation distance.
	*
	* The multiplier is a multiplicative coefficient for the floor noise.
	*
	* Floor noise only depends on (x, y) to make sure there are no holes. The scale factor is a
	* multiplier for the X,Y coords before they are provided to the Perlin noise function.
	*
	* Value of d
	*/
	float floor_att_dist;
	float cave_height;
	float cave_end_height;
	float terrace_period;

	perlin_noise_params floor_perlin;
	perlin_noise_params cave_perlin;

	field_function_structure();
};