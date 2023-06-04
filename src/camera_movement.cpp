#include <random>
#include "camera_movement.hpp"

using namespace cgp;

camera_movement_structure::camera_movement_structure()
{
	cinematic_mode = false;
	vertical_speed = 0;
	side_speed = 0;
	front_speed = 0;

	acceleration = .05f;
	maximum_movement_speed = 2.0f;
	default_movement_speed = 3.0f;
	epsilon = 1.0e-6f;
}

void camera_movement_structure::update(camera_controller_orbit_euler& camera_control)
{

	// Cinematic mode, camera movement is smoothened out
	// *************************************************************** //
	if (cinematic_mode) {

		if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_W))
			accelerate(front_speed, true);
		else if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_S))
			accelerate(front_speed, false);
		else
			deccelerate(front_speed);

		if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_A))
			accelerate(side_speed, true);
		else if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_D))
			accelerate(side_speed, false);
		else
			deccelerate(side_speed);

		if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_SPACE))
			accelerate(vertical_speed, true);
		else if (camera_control.inputs->keyboard.shift)
			accelerate(vertical_speed, false);
		else
			deccelerate(vertical_speed);

		apply_movement(camera_control);
		return;
	}

	// Default movement
	// *************************************************************** //
	if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_W))
		camera_control.camera_model.manipulator_translate_front(-default_movement_speed);
	else if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_S))
		camera_control.camera_model.manipulator_translate_front(default_movement_speed);

	if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_A))
		camera_control.camera_model.manipulator_translate_in_plane(vec2(default_movement_speed, 0));
	else if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_D))
		camera_control.camera_model.manipulator_translate_in_plane(vec2(-default_movement_speed, 0));

	if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_SPACE))
		camera_control.camera_model.manipulator_translate_in_plane(vec2(0, -default_movement_speed));
	else if (camera_control.inputs->keyboard.shift)
		camera_control.camera_model.manipulator_translate_in_plane(vec2(0, default_movement_speed));
}

void camera_movement_structure::apply_movement(cgp::camera_controller_orbit_euler& camera_control)
{
	camera_control.camera_model.manipulator_translate_front(-front_speed);
	camera_control.camera_model.manipulator_translate_in_plane(vec2(side_speed, -vertical_speed));
}

void camera_movement_structure::accelerate(float& speed, bool positive)
{
	speed += acceleration * (positive ? 1.0f : -1.0f);
	speed = positive ? std::min(maximum_movement_speed, speed) : std::max(-maximum_movement_speed, speed);
}

void camera_movement_structure::deccelerate(float& speed)
{
	if (std::abs(speed) < epsilon) return;

	bool const positive = speed > 0.0f;
	speed += acceleration * (positive ? -1.0f : 1.0f);
	speed = positive ? std::max(0.0f, speed) : std::min(0.0f, speed);
}


