#include "environment.hpp"

std::string project::path = "";
float project::gui_scale = 1.5f;

float deg_to_rad(float deg) {
	return deg * 3.14159265f / 180.0f;
}

gerstner_wave::gerstner_wave() {
	gerstner_wave(vec2(0, 0), 0, 0, 0, 0);
}

gerstner_wave::gerstner_wave(vec2 direction_, float amplitude_, float steepness_, float frequency_, float speed_) {
	direction = direction_;
	amplitude = amplitude_;
	steepness = steepness_;
	frequency = frequency_;
	speed = speed_;
}

environment_structure::environment_structure() {
	gerstner_waves_number = 3;
	gerstner_waves = new gerstner_wave[gerstner_waves_number];

	// Wave parameters
	float angle_init = deg_to_rad(25.0f);
	float angle_diff = deg_to_rad(157.45f);
	float amplitude_init = 0.5f;
	float amplitude_persistance = 0.6f;
	float steepness_init = 0.1f;
	float steepness_persistance = 1.6f;
	float frequency_init = 0.01f;
	float frequency_gain = 2.0f;
	float speed_init = 0.5f;
	float speed_gain = 1.3f;

	// Proceduraly generated wave parameters like Perlin noise
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

environment_structure::~environment_structure() {
	for (int i = 0; i < gerstner_waves_number; i++)
		delete& gerstner_waves[i];
}

void environment_structure::send_opengl_uniform(opengl_shader_structure const& shader, bool expected) const
{

	// Get camera location
	vec4 last_col_4 = camera_view * vec4(0.0, 0.0, 0.0, 1.0); // get the last column
	vec3 last_col = { last_col_4.x, last_col_4.y, last_col_4.z };
	vec3 camera_position = -transpose(mat3(camera_view)) * last_col; // get the orientation matrix * last camera view column
	opengl_uniform(shader, "camera_position", camera_position, expected);

	// Get camera direction
	vec3 camera_direction = vec3(camera_view(2, 0), camera_view(2, 1), camera_view(2, 2));
	opengl_uniform(shader, "camera_direction", camera_direction, expected);

	// Gertsner waves
	for (int i = 0; i < gerstner_waves_number; i++) {
		gerstner_wave* p_wave = &gerstner_waves[i];
		opengl_uniform(shader, "gerstner_waves[" + std::to_string(i) + "].direction", p_wave->direction, expected);
		opengl_uniform(shader, "gerstner_waves[" + std::to_string(i) + "].amplitude", p_wave->direction, expected);
		opengl_uniform(shader, "gerstner_waves[" + std::to_string(i) + "].steepness", p_wave->direction, expected);
		opengl_uniform(shader, "gerstner_waves[" + std::to_string(i) + "].frequency", p_wave->direction, expected);
		opengl_uniform(shader, "gerstner_waves[" + std::to_string(i) + "].speed", p_wave->direction, expected);
	}

	// Other uniforms
	opengl_uniform(shader, "projection", camera_projection, expected);
	opengl_uniform(shader, "view", camera_view, expected);
	opengl_uniform(shader, "light", light_color, expected);

	uniform_generic.send_opengl_uniform(shader, false);
}