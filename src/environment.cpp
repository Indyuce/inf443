#include "environment.hpp"

std::string project::path = "";
float project::gui_scale = 1.5f;

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

	// Other uniforms
	opengl_uniform(shader, "projection", camera_projection, expected);
	opengl_uniform(shader, "view", camera_view, expected);
	opengl_uniform(shader, "light", light_color, expected);
	glGetError();

	uniform_generic.send_opengl_uniform(shader, false);
}