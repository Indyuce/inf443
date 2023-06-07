#pragma once

#include "cgp/cgp.hpp"

struct camera_movement_structure {

	bool cinematic_mode = true;
	float vertical_speed, side_speed, front_speed;

	// Dev parameters
	float maximum_movement_speed, acceleration, default_movement_speed, epsilon;

	camera_movement_structure();

	void update(cgp::camera_controller_orbit_euler& camera_control);

	void accelerate(float& speed, bool positive);

	void deccelerate(float& speed);

	void apply_movement(cgp::camera_controller_orbit_euler& camera_control);
};