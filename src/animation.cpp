#include "animation.hpp"
#include <random>
using namespace cgp;

fish_manager::fish_manager()
{
	this->alignement_coef = 0.01f;
	this->cohesion_coef = 0.02f;
	this->separation_coef = 0.04f;
	this->fish_radius = 10.0f;
	this->fish_speed = 0.15f;
	this->obstacle_radius = 4.0f;
	this->obstacle_coef = 0.04f;
	this->num_group = 10;
	this->min_alga_per_group = 15;
	this->max_alga_per_group = 30;
	this->grid_step = 10;
}

void fish_manager::refresh(field_function_structure field, float t)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(0, 1);
	refresh_grid();
	counter++;
	// cgp::grid_3D<float> grid = *terrain_data->grid;
	for (int i = 0; i < fishes.size(); i++)
	{
		fish& current = fishes[i];
		// Offset of one to be able to calculate the gradient.
		int posX = (int)(current.position.x + XY_LENGTH / 2);
		posX = posX < 0 ? 0 : posX >= XY_LENGTH - 1 ? XY_LENGTH - 2
													: posX;
		int posY = (int)(current.position.y + XY_LENGTH / 2);
		posY = posY < 0 ? 0 : posY >= XY_LENGTH - 1 ? XY_LENGTH - 2
													: posY;
		int posZ = (int)(current.position.z + Z_LENGTH / 2);
		posZ = posZ < 0 ? 0 : posZ >= Z_LENGTH - 1 ? Z_LENGTH - 2
												   : posZ;
		// A positive field value means that the fish is inside a wall.
		if (field(vec3(posX, posY, posZ)) > -obstacle_radius)
		{
			float val = field(vec3(posX, posY, posZ));
			// std::cout << val << std::endl;

			// vec3 grad = cgp::normalize(vec3{field(vec3(posX+1,posY,posZ))-val,field(vec3(posX,posY+1,posZ))-val,field(vec3(posX,posY,posZ+1))-val});
			// fish.direction-= grad/(-val)*obstacle_coef;
		}
		cgp::vec3 separation = calculate_separation(current);
		cgp::vec3 alignement = calculate_alignement(current);
		cgp::vec3 cohesion = calculate_cohesion(current);
		cgp::vec3 out_of_bound_force = calculate_out_of_bound_force(current);
		current.direction += separation;
		current.direction += alignement;
		current.direction += cohesion;
		current.direction += out_of_bound_force;
		if (counter % 10 == 0)
			current.direction += {0.05 * (distrib(gen) - 0.5), 0.05 * (distrib(gen) - 0.5), 0.05 * (distrib(gen) - 0.5)};
		current.direction.z *= 0.99f;
		current.direction = cgp::normalize(current.direction);
		current.position = current.position + (fish_speed * current.direction);
	}
}

void fish_manager::refresh_grid()
{
	
	// Recreate an empty grid
	fish_grid = new std::vector<fish> **[2 * XY_LENGTH / grid_step];
	for (int i = 0; i < 2 * XY_LENGTH / grid_step; ++i)
	{
		fish_grid[i] = new std::vector<fish> *[2 * XY_LENGTH / grid_step];
		for (int j = 0; j < 2 * XY_LENGTH / grid_step; ++j)
		{
			fish_grid[i][j] = new std::vector<fish>[Z_LENGTH / grid_step];
			for (int k = 0; k < Z_LENGTH/grid_step; k++)
			{
				fish_grid[i][j][k] = std::vector<fish>();
			}
		}
	}
	for (fish f : fishes)
	{
		int posX = (int)(f.position.x + XY_LENGTH / 2);
		posX = posX < 0 ? 0 : posX >= XY_LENGTH - 1 ? XY_LENGTH - 2
													: posX;
		int posY = (int)(f.position.y + XY_LENGTH / 2);
		posY = posY < 0 ? 0 : posY >= XY_LENGTH - 1 ? XY_LENGTH - 2
													: posY;
		int posZ = (int)(f.position.z + Z_LENGTH / 2);
		posZ = posZ < 0 ? 0 : posZ >= Z_LENGTH - 1 ? Z_LENGTH - 2
												   : posZ;
		fish_grid[posX / grid_step][posY / grid_step][posZ / grid_step].push_back(f);
	}
}

std::vector<fish> fish_manager::get_neighboring_fishes(fish current)
{
	// Iterates over the 27 cells around the fish
	std::vector<fish> neighboring_fishes;
	int posX = (int)(current.position.x + XY_LENGTH / 2);
	posX = posX < 0 ? 0 : posX >= XY_LENGTH - 1 ? XY_LENGTH - 2
												: posX;
	int posY = (int)(current.position.y + XY_LENGTH / 2);
	posY = posY < 0 ? 0 : posY >= XY_LENGTH - 1 ? XY_LENGTH - 2
												: posY;
	int posZ = (int)(current.position.z + Z_LENGTH / 2);
	posZ = posZ < 0 ? 0 : posZ >= Z_LENGTH - 1 ? Z_LENGTH - 2
											   : posZ;
	for (int i = posX / grid_step - 1; i <= posX / grid_step + 1; i++)
	{
		for (int j = posY / grid_step - 1; j <= posY / grid_step + 1; j++)
		{
			for (int k = posZ / grid_step - 1; k <= posZ / grid_step + 1; k++)
			{
				if (i >= 0 && i < 2 * XY_LENGTH / grid_step && j >= 0 && j < 2 * XY_LENGTH / grid_step && k >= 0 && k < Z_LENGTH / grid_step)
				{
					for (fish f : fish_grid[i][j][k])
					{
						if (cgp::norm(f.position - current.position) < fish_radius)
						{
							neighboring_fishes.push_back(f);
						}
					}
				}
			}
		}
	}
	return neighboring_fishes;
}

cgp::vec3 fish_manager::calculate_separation(fish current)
{
	cgp::vec3 sum = {0, 0, 0};
	int count = 0;
	std::vector<fish> neighboring_fishes = get_neighboring_fishes(current);
	for (fish neighbor : neighboring_fishes)
	{
		cgp::vec3 delta = neighbor.position - current.position;
		if (cgp::norm(delta) >= 0.00001 && cgp::norm(delta) < fish_radius)
		{
			sum += -delta / std::pow(cgp::norm(delta), 2);
			count++;
		}
	}
	if (count == 0)
		return {0, 0, 0};
	vec3 separation = separation_coef * (sum / count);
	return separation - cgp::dot(separation, current.direction) * current.direction;
}
cgp::vec3 fish_manager::calculate_alignement(fish current)
{
	std::vector<fish> neighboring_fishes = get_neighboring_fishes(current);
	cgp::vec3 sum = {0, 0, 0};
	int modelId = current.modelId;
	int count = 0;
	for (fish neighbor: neighboring_fishes)
	{
		if (neighbor.modelId == modelId)
		{
			cgp::vec3 delta = neighbor.direction - current.direction;
			if (cgp::norm(delta) >= 0.001 && cgp::norm(delta) < fish_radius)
			{
				sum += neighbor.direction;
				count++;
			}
		}
	}
	if (count == 0)
		return {0, 0, 0};
	return alignement_coef * (sum / count);
}

cgp::vec3 fish_manager::calculate_cohesion(fish current)
{
	std::vector<fish> neighboring_fishes = get_neighboring_fishes(current);
	cgp::vec3 middle = {0, 0, 0};
	int modelId = current.modelId;
	int count = 0;
	for (fish neighbor: neighboring_fishes)
	{
		if (neighbor.modelId == modelId)
		{
			cgp::vec3 delta = neighbor.position - current.position;
			if (cgp::norm(delta) < fish_radius)
			{
				middle += neighbor.position;
				count++;
			}
		}
	}

	return cohesion_coef * ((middle / count) - current.position);
}

cgp::vec3 fish_manager::calculate_out_of_bound_force(fish fish)
{
	vec3 position = fish.position;
	float out_of_bound_force = 0.01f;
	float forceX = position.x > 100 ? -out_of_bound_force : position.x < -100 ? out_of_bound_force
																			  : 0;
	float forceY = position.y > 100 ? -out_of_bound_force : position.y < -100 ? out_of_bound_force
																			  : 0;
	float forceZ = position.z > -10 ? -out_of_bound_force : position.z < -90 ? out_of_bound_force
																			 : 0;
	return {forceX, forceY, forceZ};
}
