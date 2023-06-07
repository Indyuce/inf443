#pragma once

#include "field_function.hpp"

using namespace cgp;

perlin_noise_params::perlin_noise_params() {
    persistency = 0.0f;
    frequency_gain = 0.0f;
    octave = 0.0f;
    scale = 0.0f;
    multiplier = 0.0f;
    offset = 0.0f;

    direction = vec2(0.0f, 0.0f);
    speed = 0.0f;
    normal_boost = 0.0f;
}

perlin_noise_params::perlin_noise_params(float persistency_, float frequency_gain_, int octave_, float scale_, float multiplier_, float offset_) {
    persistency = persistency_;
    frequency_gain = frequency_gain_;
    octave = octave_;
    scale = scale_;
    multiplier = multiplier_;
    offset = offset_;

    direction = vec2(0.0f, 0.0f);
    speed = 0.0f;
    normal_boost = 0.0f;
}

perlin_noise_params::perlin_noise_params(float persistency_, float frequency_gain_, int octave_, float scale_, float multiplier_, vec2& direction_, float speed_, float normal_boost_) {
    persistency = persistency_;
    frequency_gain = frequency_gain_;
    octave = octave_;
    scale = scale_;
    multiplier = multiplier_;
    offset = 0.0f;

    direction = direction_;
    speed = speed_;
    normal_boost = normal_boost_;
}

float perlin_noise_params::compute(const cgp::vec3& pos) const {
    return noise_perlin({ pos.x * scale, pos.y * scale, pos.z * scale }, octave, persistency, frequency_gain) * multiplier - offset;
}

float perlin_noise_params::compute(const cgp::vec2& pos) const {
    return noise_perlin({ pos.x * scale, pos.y * scale }, octave, persistency, frequency_gain) * multiplier - offset;
}

float perlin_noise_params::compute(cgp::vec2 const& pos, float time) const
{
    return noise_perlin({ pos.x * scale, pos.y * scale }, octave, persistency, frequency_gain) * multiplier - offset;
}

field_function_structure::field_function_structure() {
    floor_att_dist = 2.0f;
    floor_1_level = -200.0f;
    //cave_height_1 = 30.0f;
    //cave_height_max = cave_height_1 * 2.0f;

    floor_perlin = perlin_noise_params(0.175f, 1.25f, 2, .005f, 1.0f, -2.3f);
    cave_perlin = perlin_noise_params(0.31f, 2.81f, 3, .0015f, 7.28f, 4.56f);
    //rock_color_perlin = perlin_noise_params(0.4f, 3.0f, 4, .05f, 1.0f, 0.0f);
    //mossy_rocks_perlin = perlin_noise_params(0.4f, 3.0f, 2, .05f, 10.0f, 3.0f);
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
/// <summary>
/// Unused at the moment. Volumetric color looks REALLY bad when not using a custom texture.
/// Formula to compute terrain color at a specific location
/// </summary>
/// <param name="pos"></param>
/// <returns></returns>
cgp::vec3 field_function_structure::color_at(const vec3& pos) const {

    // basalt -> iron (https://encycolorpedia.fr/575d5e https://encycolorpedia.com/a09484)
    const static vec3 color_1 = vec3(.3412, .3647, .3686);
    const static vec3 color_2 = vec3(.6275, .5804, .5176);

    const float terrain = rock_color_perlin.compute(pos);
    const float terrain_threshold = .9f;
    const float p = std::min(1.0f, terrain / terrain_threshold);
    vec3 voxel_color = p * color_1 + (1 - p) * color_2;
    // std::cout << "Noise: " << noise << " p: " << p << std::endl;

    // malachite https://encycolorpedia.fr/1fa055
    // color of mossy stone
    const static vec3 color_moss = vec3(.1216, .6275, .3333);

    // Apply some mossy stone
    const vec3 seed = vec3(42.0f, 43.0f, 44.0f);
    const float moss = mossy_rocks_perlin.compute(pos + seed);
    const float moss_threshold = 1.0f;
    const float p2 = std::min(1.0f, moss / moss_threshold);
    voxel_color = p2 * voxel_color + (1 - p2) * color_moss;

    return voxel_color;
}*/

/// <summary>
/// Formula to compute terrain potential (implicit surface) at a specific location
/// </summary>
/// <param name="pos"></param>
/// <returns></returns>
float field_function_structure::operator()(cgp::vec3 const& pos) const
{
    float pot = 0.0f;

    // Bottom hills
    float const height = pos.z - ground_level;
    float const floor_pot = floor_perlin.compute(vec2(pos.x, pos.y)) * exp(-height / floor_att_dist);
    pot += floor_pot;
    
    // Add caves
    bool const low = pos.z < floor_1_level;
    float const cave_height = floor_1_level - ground_level;
    float const mult = (0.5f + 0.6f * height / cave_height) * (low ? 1.0f : 0.9f * exp(-(pos.z - floor_1_level) * 2.0f));
    float const cave_pot = mult * cave_perlin.compute(pos);
    pot += cave_pot;
    
    return pot;
}
