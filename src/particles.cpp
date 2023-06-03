#include "particles.hpp"
#pragma once

using namespace cgp;

/// <summary>
/// Creates one particle which still needs to be registered afterwards.
/// </summary>
/// <param name="position_">Initial position</param>
/// <param name="velocity_">Initial velocity</param>
/// <param name="angle_">Initial view angle of player</param>
/// <param name="rot_speed_">Initial rotation velocity</param>
/// <param name="mass_">Particule mass. Negative mass can account for high buoyancy</param>
/// <param name="inertia_">Particle inertia (resistance to torque)</param>
/// <param name="friction_">Particle friction (can be isotropic) ie resistance to motion inside of water/air</param>
/// <param name="rot_friction_">Attenuation of rotation speed with time</param>
/// <param name="lifetime_">Particle maximum lifetime</param>
/// <param name="scale_">Particle scale, default is 1.0</param>
particle::particle(vec3& position_, vec3& velocity_, float angle_, float rot_speed_, float mass_, float inertia_, vec3& friction_, float rot_friction_, float lifetime_, float scale_)
{
	moving = true;
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
	scale = scale_;
}

// No angle for symetrical particles
particle::particle(vec3& position_, vec3& velocity_, float mass_, vec3& friction_, float lifetime_, float scale_)
{
	moving = true;
	position = position_;
	velocity = velocity_;
	angle = 0;
	rot_speed = 0;

	mass = mass_;
	inertia = 0;
	friction = friction_;
	rot_friction = 0;

	lifetime = lifetime_;
	time_lived = .0f;
	scale = scale_;
}

// Non moving particles.
particle::particle(cgp::vec3& position_, float lifetime_, float scale_)
{
	moving = false;
	position = position_;
	velocity = vec3(0, 0, 0);
	angle = 0;
	rot_speed = 0;

	mass = 0;
	inertia = 0;
	friction = vec3(0, 0, 0);
	rot_friction = 0;

	lifetime = lifetime_;
	time_lived = .0f;
	scale = scale_;
}

void particle_manager::initialize(float& t, std::string& project_path)
{
	z_limit = -30.0f;
	last_time = t;
	gravity = -9.81f * vec3(0.0f, 0.0f, 1.0f);

	opengl_shader_structure particle_shader;
	particle_shader.load(
		project_path + "shaders/particle/vert.glsl",
		project_path + "shaders/particle/frag.glsl");

	mesh quadrangle = mesh_primitive_quadrangle({ -0.5f,0,0 }, { 0.5f,0,0 }, { 0.5f,0,1 }, { -0.5f,0,1 });

	// 0 Bubble
	mesh_drawable bubble;
	bubble.initialize_data_on_gpu(quadrangle);
	bubble.shader = particle_shader;
	bubble.texture.load_and_initialize_texture_2d_on_gpu(project_path + "assets/particle/bubble1.png");
	particle_types.push_back(particle_type(bubble, 1.0f));

	// 1 Dot
	mesh_drawable dot;
	dot.initialize_data_on_gpu(quadrangle);
	dot.shader = particle_shader;
	dot.texture.load_and_initialize_texture_2d_on_gpu(project_path + "assets/particle/dot.png");
	particle_types.push_back(particle_type(dot, .3f));
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

		// Make particle die sooner if reaches Z limit
		if (p_active->position.z > z_limit)
			p_active->time_lived = std::max(p_active->lifetime - 1.0f, p_active->time_lived);
		
		// Forces and torques
		vec3 const force = gravity * p_active->mass - p_active->velocity * p_active->friction;
		double const torque = -p_active->rot_speed * p_active->rot_friction;

		// Update velocities
		p_active->velocity += force * dt * (p_active->mass != 0 ? 1.0f / std::abs(p_active->mass) : 1.0f);
		p_active->rot_speed += torque / p_active->inertia * dt;

		// Update positions
		p_active->position += p_active->velocity * dt;
		p_active->angle += p_active->rot_speed * dt;

		++i;
	}
}

void particle_manager::register_particle(particle& particle, int model_id)
{
	// Missing fields
	particle.type = &particle_types.at(model_id);

	// Add to particle registry
	active_particles.push_back(particle);
}

particle_type::particle_type(cgp::mesh_drawable& drawable_, float scale_)
{
	drawable = drawable_;
	scale = scale_;
}
