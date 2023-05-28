#include "environment.hpp"

std::string project::path = "";
float project::gui_scale = 1.5f;

float radians(float const& deg) {
	return deg * Pi / 180.0f;
}

vec3 environment_structure::get_camera_position() const
{
	vec4 last_col_4 = camera_view * vec4(0.0, 0.0, 0.0, 1.0); // get the last column
	vec3 last_col = { last_col_4.x, last_col_4.y, last_col_4.z };
	return -transpose(mat3(camera_view)) * last_col; // get the orientation matrix * last camera view column
}

vec3 compute_direction(const float& azimut, const float& polar) {
	const float azimut_rad = azimut * Pi / 180.0f;
	const float polar_rad = polar * Pi / 180.0f;
	return vec3(cos(azimut_rad) * sin(polar_rad), sin(azimut_rad) * sin(polar_rad), cos(polar_rad));
}

vec3 compute_direction(vec2 const& dir) {
	return compute_direction(dir.x, dir.y);
}

void environment_structure::send_opengl_uniform(opengl_shader_structure const& shader, bool expected) const
{

	// Phong illumination
	opengl_uniform(shader, "ambiant", ambiant, expected);
	opengl_uniform(shader, "diffuse", diffuse, expected);
	opengl_uniform(shader, "specular", specular, expected);
	opengl_uniform(shader, "specular_exp", specular_exp, expected);

	// Direct illumination
	opengl_uniform(shader, "direct", direct, expected);
	opengl_uniform(shader, "direct_exp", direct_exp, expected);

	// Light 
	opengl_uniform(shader, "light_color", light_color, expected);
	opengl_uniform(shader, "light_direction", compute_direction(light_direction), expected);
	// environment.uniform_generic.uniform_float["attenuation_distance"] = environment.attenuation_distance;

	// Player flashlight
	opengl_uniform(shader, "flashlight", flashlight, expected);
	opengl_uniform(shader, "flashlight_exp", flashlight_exp, expected);
	opengl_uniform(shader, "flashlight_exp", flashlight_exp, expected);

	// Water & Attenuation
	opengl_uniform(shader, "fog_color", fog_color, expected);
	opengl_uniform(shader, "surf_height", surf_height, expected);
	opengl_uniform(shader, "floor_level", floor_level, expected);
	opengl_uniform(shader, "scale", scale, expected);
	opengl_uniform(shader, "water_attenuation_coefficient", water_attenuation_coefficient, expected);

	// Extra
	opengl_uniform(shader, "projection", camera_projection, expected);
	opengl_uniform(shader, "view", camera_view, expected);
	opengl_uniform(shader, "ridge_coefficient", terrain_ridges, expected);
	opengl_uniform(shader, "water_optical_index", water_optical_index, expected);
	opengl_uniform(shader, "sand_texture_scale", sand_texture_scale, expected);

	// Extra uniforms
	uniform_generic.send_opengl_uniform(shader, false);
}

// CODE DUPLICATION
static int const gerstner_waves_number = 3;
static bool filled = false;
static gerstner_wave gerstner_waves[gerstner_waves_number];
static perlin_noise_params surface_ridges = perlin_noise_params(0.8f, 1.3f, 7, .02f, 1.0f, vec2(1, 1), 0.5f, 10.0f);

// CODE DUPLICATION
vec3 gerstner_wave_position(vec3 const& position, float& time) {
	vec3 wave_position = position;
	vec2 xy = vec2(position.x, position.y);

	// Big waves
	for (int i = 0; i < gerstner_waves_number; ++i) {
		float proj = dot(xy, gerstner_waves[i].direction),
			phase = time * gerstner_waves[i].speed,
			theta = proj * gerstner_waves[i].frequency + phase,
			height = gerstner_waves[i].amplitude * sin(theta);

		wave_position.z += height;

		float maximum_width = gerstner_waves[i].steepness *
			gerstner_waves[i].amplitude,
			width = maximum_width * cos(theta),
			x = gerstner_waves[i].direction.x,
			y = gerstner_waves[i].direction.y;

		wave_position.x += x * width;
		wave_position.y += y * width;
	}

	// Surface ridges
	wave_position.z += surface_ridges.compute(xy, time);

	return wave_position;
}

// CODE DUPLICATION
float environment_structure::get_water_level(vec3 const& position, float& time) const
{
	if (!filled) {
		filled = true;
		// Proceduraly generated wave parameters like Perlin noise
		// TODO convert waves to object buffer to avoid code duplication
		float angle_init = radians(25.0f);
		float angle_diff = radians(157.45f);
		float amplitude_init = 0.5f;
		float amplitude_persistance = 0.6f;
		float steepness_init = 0.1f;
		float steepness_persistance = 1.6f;
		float frequency_init = 0.1f;
		float frequency_gain = 2.0f;
		float speed_init = 0.5f;
		float speed_gain = 1.3f;

		for (int i = 0; i < gerstner_waves_number; ++i) {
			vec2 dir = vec2(cos(angle_init), sin(angle_init));
			gerstner_waves[i] = gerstner_wave(dir, amplitude_init, steepness_init, frequency_init, speed_init);

			angle_init += angle_diff;
			amplitude_init *= amplitude_persistance;
			steepness_init *= steepness_persistance;
			frequency_init *= frequency_gain;
			speed_init *= speed_gain;
		}
	}
	

	return gerstner_wave_position(position, time).z;
}
