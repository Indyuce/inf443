#pragma once

#include "cgp/cgp.hpp"

struct particle_type {
	cgp::mesh_drawable drawable;
	float scale;

	particle_type(cgp::mesh_drawable& drawable_, float scale_);
};

struct particle {

	// Cinematic
	cgp::vec3 velocity, position;
	float rot_speed, angle;
	bool moving;

	// Force parameters
	float mass, inertia;
	cgp::vec3 friction;
	float rot_friction;

	// Generic parameters
	float lifetime, time_lived, scale;
	particle_type* type; // Initialized when registered

	particle(cgp::vec3& position_, float lifetime_, float scale_);

	particle(cgp::vec3& position_, cgp::vec3& velocity_, float angle_, float rot_speed_, float mass_, float inertia_, cgp::vec3& friction_, float rot_friction_, float lifetime_, float scale_);
	
	particle(cgp::vec3& position_, cgp::vec3& velocity_, float mass_, cgp::vec3& friction_, float lifetime_, float scale_);
};

struct particle_manager
{
	float last_time, z_limit;
	cgp::vec3 gravity;
	std::vector<particle> active_particles;
	std::vector<particle_type> particle_types;

	void initialize(float& t, std::string& project_path);

	void tick(float& dt);

	void register_particle(particle& particle, int model_id);
};

