#pragma once

#include "terrain.hpp"

const float terrain::SCALE = 1.0f / (float)terrain::PRECISION;
const float terrain::ISOVALUE = 0.0f;

/// <summary>
/// Uses marching cubes from CGP to compute data of a specific chunk.
/// </summary>
/// <param name="chunk_x"></param>
/// <param name="chunk_y"></param>
/// <returns></returns>
chunk_data* terrain::generate_chunk_data(int chunk_x, int chunk_y, opengl_shader_structure& shader) {

    // Initialize empty chunk data
    chunk_data* data = new chunk_data();

    // Initialize the spatial domain and the field
    spatial_domain_grid_3D domain = spatial_domain_grid_3D::from_center_length({ 0,0,0 }, { XY_LENGTH, XY_LENGTH, Z_LENGTH }, { XY_SAMPLES, XY_SAMPLES, Z_SAMPLES });
    grid_3D<float> field = compute_scalar_field(domain);
    

    // Compute mesh using and initialize drawable
    mesh chunk_mesh = marching_cube(field, domain, ISOVALUE);
    data->initialize(chunk_mesh,field, shader);

    return data;
}

/// <summary>
/// Compures 3D scalar field of potential
/// </summary>
/// <param name="domain"></param>
/// <returns></returns>
grid_3D<float> terrain::compute_scalar_field(spatial_domain_grid_3D const& domain)
{
    grid_3D<float> field;
    field.resize(domain.samples);

    // Fill the discrete field values
    for (int kz = 0; kz < domain.samples.z; kz++) 
        for (int ky = 0; ky < domain.samples.y; ky++) 
            for (int kx = 0; kx < domain.samples.x; kx++) {

                vec3 const p = domain.position({ kx, ky, kz });
                field(kx, ky, kz) = potential(domain, p);
            }

    return field;
}

/// <summary>
/// https://www.johndcook.com/blog/2010/01/20/how-to-compute-the-soft-maximum/
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <returns>Soft maximum between x and y</returns>
float soft_max(float x, float y)
{
    const float maximum = std::max(x, y);
    const float minimum = std::min(x, y);
    return maximum + log(1.0 + exp(minimum - maximum));
}


/// <summary>
/// 
/// </summary>
/// <param name="x">Can be positive and negative</param>
/// <param name="y">Must be positive</param>
/// <returns>x % y</returns>
float mod(float x, float y) {
    while (x < 0)
        x += y;
    while (x >= y)
        x -= y;
    return x;
}

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
float const FLOOR_ATTENUATION_DISTANCE = 10.0f;
float const TERRACE_PERIOD = .1f;

perlin_noise_parameters const FLOOR_PERLIN = perlin_noise_parameters(0.3f, 2.0f, 2, .01f, 1.0f);

perlin_noise_parameters const CAVES = perlin_noise_parameters(0.3f, 2.0f, 3, .03f, 8.0f);
float const CAVES_HEIGHT = 20.0f;
float const CAVES_HEIGHT_2 = CAVES_HEIGHT * 1.5f;

/// <summary>
/// 
/// </summary>
/// <param name="domain"></param>
/// <param name="pos"></param>
/// <returns>Terrain potential at a given position</returns>
float terrain::potential(spatial_domain_grid_3D const& domain, vec3 const& pos) {
    float pot = 0.0f;

    const float x = pos.x;
    const float y = pos.y;
    const float z = pos.z - domain.corner_min().z ;

    // Bottom hills
    //float const floor_pot = noise_perlin({ x * FLOOR_PERLIN.scale, y * FLOOR_PERLIN.scale }, FLOOR_PERLIN.octave, FLOOR_PERLIN.persistency, FLOOR_PERLIN.frequency_gain);
    //pot += floor_pot * FLOOR_PERLIN.multiplier * exp(-z / FLOOR_ATTENUATION_DISTANCE) - 3.0f;

    const float TERRACE_HEIGHT = 20;

    // Caves with terraces
    const float period_mult = 1.0f + .007f * z;
    const float floor_1_pot = noise_perlin({ x * CAVES.scale, y * CAVES.scale, z * CAVES.scale }, CAVES.octave, CAVES.persistency, CAVES.frequency_gain) * CAVES.multiplier;
    pot += period_mult * floor_1_pot - 10.0f;

   // std::cout << z << " -> " << pot << std::endl;

    // Make sure the floor is always displayed
    //if (z < 3.0f) pot = soft_max(0.1f, pot);
   // std::cout << z << " " << pot << std::endl;

    return pot;

    /*
    // Caves on the bottom
    const float floor_1_pot = noise_perlin({ x * CAVES.scale, y * CAVES.scale , z * CAVES.scale }, CAVES.octave, CAVES.persistency, CAVES.frequency_gain) * CAVES.multiplier;
    float floor_1_mult = (z > CAVES_HEIGHT ? .8f : 1.0f) * (z > CAVES_HEIGHT_2 ? exp(-(z - CAVES_HEIGHT_2) / 20.0f) : 1.0f);
    pot += floor_1_pot * floor_1_mult; 

    return pot - 2.7f;*/
}