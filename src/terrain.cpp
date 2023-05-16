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
                field(kx, ky, kz) = 0;//
            }

    return field;
}