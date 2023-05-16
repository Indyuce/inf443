#include "field_function.hpp"

using namespace cgp;

perlin_noise_params::perlin_noise_params(float persistency_, float frequency_gain_, int octave_, float scale_, float multiplier_, float offset_) {
    persistency = persistency_;
    frequency_gain = frequency_gain_;
    octave = octave_;
    scale = scale_;
    multiplier = multiplier_;
    offset = offset_;
}

perlin_noise_params::perlin_noise_params() {
    persistency = 0.0f;
    frequency_gain = 0.0f;
    octave = 0;
    scale = 0.0f;
    scale = 0.0f;
    offset = 0.0f;
}

field_function_structure:: field_function_structure() {
    floor_att_dist = 10.0f;
    cave_height_1 = 30.0f;
    cave_height_max = cave_height_1 * 2.0f;

    floor_perlin = perlin_noise_params(0.3f, 2.0f, 2, .01f, 1.0f, .3f);
    cave_perlin = perlin_noise_params(0.307f, 2.193f, 3, .03f, 5.47f, 4.6f);
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

float soft_min(float x, float y) {
    return -soft_max(-x, -y);
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

cgp::vec3 field_function_structure::color_at(cgp::vec3 const& pos) const {
    return { 0.91f, 0.6f, 0.17f };
}

float field_function_structure::operator()(cgp::vec3 const& pos) const
{
    float pot = 0.0f;

    const float x = pos.x;
    const float y = pos.y;
    const float MIN_Z = -25;
    const float z = pos.z - MIN_Z; // <chunk height>/2
    //std::cout << z << std::endl;

    // Bottom hills
    float const floor_pot = noise_perlin({ x * floor_perlin.scale, y * floor_perlin.scale }, floor_perlin.octave, floor_perlin.persistency, floor_perlin.frequency_gain);
    pot += floor_pot * floor_perlin.multiplier * exp(-z / floor_att_dist) - floor_perlin.offset;

    // Add caves
    if (z < cave_height_max) {
        bool const low = z < cave_height_1;
        float cave_pot = noise_perlin({ x * cave_perlin.scale, y * cave_perlin.scale, z * cave_perlin.scale }, cave_perlin.octave, cave_perlin.persistency, cave_perlin.frequency_gain * (low ? 1.0f : 1.3f));
        cave_pot = cave_pot * cave_perlin.multiplier * (low ? 1 : 0.9f * exp(-(z - cave_height_1) / 20.0f)) - cave_perlin.offset;
        pot = soft_max(cave_pot, pot);
    }

    // Make sure the floor is always displayed
    if (z < .01) pot = soft_max(0.001f, pot);

    return pot;
}
