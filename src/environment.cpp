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

	// Direct illumination
	opengl_uniform(shader, "direct", direct, expected);
	opengl_uniform(shader, "direct_exp", direct_exp, expected);

	// Light
	opengl_uniform(shader, "light_color", light_color, expected);
	opengl_uniform(shader, "light_direction", compute_direction(light_direction), expected);
	opengl_uniform(shader, "fog_distance", fog_distance, expected);

	// Player flashlight
	opengl_uniform(shader, "flashlight_on", flashlight_on, expected);
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
