
#include "terrain.hpp"


using namespace cgp;

// Evaluate 3D position of the terrain for any (x,y)
float evaluate_terrain_height(float x, float y, float terrain_length, perlin_noise_parameters parameters)
{
    float u = x / terrain_length;
    float v = y / terrain_length;

    // Compute the Perlin noise
    return evaluate_terrain_height(u, v, parameters);
}

// Evaluate 3D position of the terrain for any (x,y)
float evaluate_terrain_height(float u, float v, perlin_noise_parameters parameters)
{

    // Compute the Perlin noise
    float const noise = noise_perlin({ u, v }, parameters.octave, parameters.persistency, parameters.frequency_gain);

    // use the noise as height value
    return parameters.terrain_height * noise;
}

std::vector<cgp::vec3> generate_positions_on_terrain(int N, float terrain_length, perlin_noise_parameters parameters) {

    std::vector<cgp::vec3> vec = {};

    for (int i = 0; i < N; i++) {
        float x = rand_interval() * terrain_length;
        float y = rand_interval() * terrain_length;

        vec.push_back(vec3{x,y,0});
    }

    return vec;

}

mesh create_terrain_mesh(int N, float terrain_length)
{
    mesh terrain = mesh_primitive_grid({ 0,0,0 }, { terrain_length,0,0 }, { terrain_length,terrain_length,0 }, { 0,terrain_length,0 }, N, N);
    terrain.uv.resize(N * N);

    // Fill terrain geometry
    for (int ku = 0; ku < N; ++ku)
    {
        for (int kv = 0; kv < N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            float u = ku / (N - 1.0f);
            float v = kv / (N - 1.0f);

            // Store vertex coordinates
            terrain.uv[kv + N * ku] = { 5 * u, 5 * v };
        }
    }

    return terrain;
}

/*
mesh create_terrain_mesh(int N, float terrain_length)
{

    mesh terrain; // temporary terrain storage (CPU only)
    terrain.position.resize(N*N);
    terrain.uv.resize(N * N);

    // Fill terrain geometry
    for(int ku=0; ku<N; ++ku)
    {
        for(int kv=0; kv<N; ++kv)
        {
            // Compute local parametric coordinates (u,v) \in [0,1]
            float u = ku/(N-1.0f);
            float v = kv/(N-1.0f);

            // Compute the real coordinates (x,y) of the terrain in [-terrain_length/2, +terrain_length/2]
            float x = (u - 0.5f) * terrain_length;
            float y = (v - 0.5f) * terrain_length;

            // Compute the surface height function at the given sampled coordinate
            float z = evaluate_terrain_height(x,y);

            // Store vertex coordinates
            terrain.position[kv+N*ku] = {x,y,z};
            terrain.uv[kv + N * ku] = { 5 * u, 5 *  v };
        }
    }

    // Generate triangle organization
    //  Parametric surface with uniform grid sampling: generate 2 triangles for each grid cell
    for(int ku=0; ku<N-1; ++ku)
    {
        for(int kv=0; kv<N-1; ++kv)
        {
            unsigned int idx = kv + N*ku; // current vertex offset

            uint3 triangle_1 = {idx, idx+1+N, idx+1};
            uint3 triangle_2 = {idx, idx+N, idx+1+N};

            terrain.connectivity.push_back(triangle_1);
            terrain.connectivity.push_back(triangle_2);
        }
    }

    // need to call this function to fill the other buffer with default values (normal, color, etc)
	terrain.fill_empty_field(); 

    return terrain;
}*/

void update_terrain(mesh& terrain, mesh_drawable& terrain_visual, perlin_noise_parameters const& parameters)
{
    // Number of samples in each direction (assuming a square grid)
    int const N = std::sqrt(terrain.position.size());

    // Recompute the new vertices
    for (int ku = 0; ku < N; ++ku) {
        for (int kv = 0; kv < N; ++kv) {

            // Compute local parametric coordinates (u,v) \in [0,1]
            const float u = ku / (N - 1.0f);
            const float v = kv / (N - 1.0f);

            int const idx = ku * N + kv;

            // use the noise as height value
            terrain.position[idx].z = evaluate_terrain_height(u, v, parameters);

            // use also the noise as color value
           // terrain.color[idx] = 0.3f * vec3(0, 0.5f, 0) + 0.7f * noise * vec3(1, 1, 1);
        }
    }

    // Update the normal of the mesh structure
    terrain.normal_update();

    // Update step: Allows to update a mesh_drawable without creating a new one
    terrain_visual.vbo_position.update(terrain.position);
    terrain_visual.vbo_normal.update(terrain.normal);
    terrain_visual.vbo_color.update(terrain.color);

}