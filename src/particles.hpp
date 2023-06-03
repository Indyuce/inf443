#pragma once

#include "cgp/cgp.hpp"

struct particle {

	// Cinematic
	cgp::vec3 velocity, position;
	float rot_speed, angle;

	// Force parameters
	float mass, inertia;
	cgp::vec3 friction;
	float rot_friction;

	// Generic parameters
	float lifetime, time_lived;
	int model_id;

	particle(cgp::vec3& velocity_, cgp::vec3& position_, float rot_speed_, float angle_, float mass_, float inertia_, cgp::vec3& friction_, float rot_friction_, float lifetime_, int model_id_);
};

struct particle_manager
{
	

	float last_time;
	cgp::vec3 gravity;
	std::vector<particle> active_particles;
	std::vector<cgp::mesh_drawable> particle_types;

	void initialize(float& t, std::string& project_path);

	void tick(float& dt);

	void register_particle(particle& particle);
};

