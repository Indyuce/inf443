#include "animation.hpp"
#include <random>
using namespace cgp;

fish_manager::fish_manager()
{
	ticks = 0;
	fish_groups_number = 5;
	fishes_per_group = 20;

	this->alignement_coef = 0.01f;
	this->cohesion_coef = 0.02f;
	this->separation_coef = 0.4f;
	this->fish_radius = 10.0f;
	this->fish_speed = 0.15f;
	this->obstacle_radius = 4.0f;
	this->obstacle_coef = 0.04f;
	this->num_group = 10;
	this->min_alga_per_group = 15;
	this->max_alga_per_group = 30;
	this->grid_step = 10;
}

void fish_manager::initialize(vec3 domain, std::string project_path) {
	domain_x = domain.x;
	domain_y = domain.y;
	domain_z = domain.z;

	float scales[5] = { 1.5f, 1.5f, 3.0f, 1.5f, 12.0f };

	for (int i = 0; i < 5; i++) {
		cgp::mesh_drawable drawable;

		std::string path = std::to_string(i);
		drawable.initialize_data_on_gpu(mesh_load_file_obj(project_path + "assets/fish" + path + "/fish" + path + ".obj"));
		drawable.texture.load_and_initialize_texture_2d_on_gpu(project_path + "assets/fish" + path + "/fish" + path + ".png");
		opengl_shader_structure drawable_shader;
		drawable_shader.load(
			project_path + "shaders/fish" + path + "/vert.glsl",
			project_path + "shaders/terrain/frag.glsl");
		drawable.shader = drawable_shader;
		drawable.model.scaling = scales[i];

		fish_models.push_back(drawable);
	}

	alga_model.initialize_data_on_gpu(mesh_load_file_obj(project_path + "assets/alga/alga.obj"));
	alga_model.texture.load_and_initialize_texture_2d_on_gpu(project_path + "assets/alga/alga.jpeg");
	opengl_shader_structure alga_shader;
	alga_shader.load(
		project_path + "shaders/alga/vert.glsl",
		project_path + "shaders/terrain/frag.glsl");
	alga_model.shader = alga_shader;
	alga_model.model.scaling = 4.0f;
}

void fish_manager::refresh(field_function_structure field, float t)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(0, 1);
	refresh_grid();
	ticks = ++ticks % 10;
	// cgp::grid_3D<float> grid = *terrain_data->grid;
	for (int i = 0; i < fishes.size(); i++)
	{
		fish& current = fishes[i];
		// Offset of one to be able to calculate the gradient.
		int posX = (int)(current.position.x + domain_x / 2);
		posX = posX < 0 ? 0 : posX >= domain_x - 1 ? domain_x - 2
													: posX;
		int posY = (int)(current.position.y + domain_y / 2);
		posY = posY < 0 ? 0 : posY >= domain_y - 1 ? domain_y - 2
													: posY;
		int posZ = (int)(current.position.z + domain_z / 2);
		posZ = posZ < 0 ? 0 : posZ >= domain_z - 1 ? domain_z - 2
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
		if (ticks % 10 == 0)
			current.direction += {0.05 * (distrib(gen) - 0.5), 0.05 * (distrib(gen) - 0.5), 0.05 * (distrib(gen) - 0.5)};
		current.direction.z *= 0.99f;
		current.direction = cgp::normalize(current.direction);
		current.position = current.position + (fish_speed * current.direction);
	}
}

void fish_manager::refresh_grid()
{
	
	// Recreate an empty grid
	fish_grid = new std::vector<fish> **[2 * domain_x / grid_step];
	for (int i = 0; i < 2 * domain_x / grid_step; ++i)
	{
		fish_grid[i] = new std::vector<fish> *[2 * domain_y / grid_step];
		for (int j = 0; j < 2 * domain_y / grid_step; ++j)
		{
			fish_grid[i][j] = new std::vector<fish>[domain_z / grid_step];
			for (int k = 0; k < domain_z /grid_step; k++)
			{
				fish_grid[i][j][k] = std::vector<fish>();
			}
		}
	}
	for (fish f : fishes)
	{
		int posX = (int)(f.position.x + domain_x / 2);
		posX = posX < 0 ? 0 : posX >= domain_x - 1 ? domain_x - 2
													: posX;
		int posY = (int)(f.position.y + domain_y / 2);
		posY = posY < 0 ? 0 : posY >= domain_y - 1 ? domain_y - 2
													: posY;
		int posZ = (int)(f.position.z + domain_z / 2);
		posZ = posZ < 0 ? 0 : posZ >= domain_z - 1 ? domain_z - 2
												   : posZ;
		fish_grid[posX / grid_step][posY / grid_step][posZ / grid_step].push_back(f);
	}
}

std::vector<fish> fish_manager::get_neighboring_fishes(fish current)
{
	// Iterates over the 27 cells around the fish
	std::vector<fish> neighboring_fishes;
	int posX = (int)(current.position.x + domain_x / 2);
	posX = posX < 0 ? 0 : posX >= domain_x - 1 ? domain_x - 2
												: posX;
	int posY = (int)(current.position.y + domain_y / 2);
	posY = posY < 0 ? 0 : posY >= domain_y - 1 ? domain_y - 2
												: posY;
	int posZ = (int)(current.position.z + domain_z / 2);
	posZ = posZ < 0 ? 0 : posZ >= domain_z - 1 ? domain_z - 2
											   : posZ;
	for (int i = posX / grid_step - 1; i <= posX / grid_step + 1; i++)
	{
		for (int j = posY / grid_step - 1; j <= posY / grid_step + 1; j++)
		{
			for (int k = posZ / grid_step - 1; k <= posZ / grid_step + 1; k++)
			{
				if (i >= 0 && i < 2 * domain_x / grid_step && j >= 0 && j < 2 * domain_y / grid_step && k >= 0 && k < domain_z / grid_step)
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
