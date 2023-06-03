#include "particles.hpp"
#pragma once

using namespace cgp;

particle::particle(vec3& position_, vec3& velocity_, float angle_, float rot_speed_, float mass_, float inertia_, vec3& friction_, float rot_friction_, float lifetime_, int model_id_)
{
	position = position_;
	velocity = velocity_;
	angle = angle_;
	rot_speed = rot_speed_;

	mass = mass_;
	inertia = inertia_;
	friction = friction_;
	rot_friction = rot_friction_;

	lifetime = lifetime_;
	time_lived = .0f;
	model_id = model_id_;

	rand();
}

void particle_manager::initialize(float& t, std::string& project_path)
{
	last_time = t;
	gravity = -9.81f * vec3(0.0f, 0.0f, 1.0f);
}

void particle_manager::tick(float& t)
{
	// Time
	float const dt = t - last_time;
	last_time = t;

	int i = 0;
	while (i < active_particles.size()) {
		particle* p_active = &active_particles.at(i);

		// Update time and unregister if needed
		p_active->time_lived += dt;
		if (p_active->time_lived > p_active->lifetime) {
			active_particles.erase(active_particles.begin() + i);
			continue;
		}
		
		// Forces and torques
		vec3 const force    = gravity * p_active->mass - p_active->velocity * p_active->friction;
		double const torque = -p_active->rot_speed * p_active->rot_friction;

		// Update velocities
		p_active->velocity  += force / p_active->mass * dt;
		p_active->rot_speed += torque / p_active->inertia * dt;

		// Update positions
		p_active->position += p_active->velocity * dt;
		p_active->angle    += p_active->rot_speed * dt;

		++i;
	}
}

void particle_manager::register_particle(particle& particle)
{
	active_particles.push_back(particle);
}
