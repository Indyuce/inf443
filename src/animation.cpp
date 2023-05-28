#include "animation.hpp"
#include <random>
using namespace cgp;

fish_manager::fish_manager() {
	this->alignement_coef = 0.01f;
	this->cohesion_coef = 0.02f;
	this->separation_coef = 0.04f;
	this->fish_radius = 10.0f;
	this->fish_speed = 0.15f;
	this->obstacle_radius = 4.0f;
	this->obstacle_coef = 0.04f;
}

void fish_manager::add(fish fish) {
	fishes.push_back(fish);
}

void fish_manager::refresh(field_function_structure field) {
	counter++;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(0, 1);
	//cgp::grid_3D<float> grid = *terrain_data->grid;
	for (int i = 0;i < fishes.size();i++) {
		//Offset of one to be able to calculate the gradient.
		int posX = (int)(fishes[i].position.x+XY_LENGTH/2);
		posX = posX < 0 ? 0 : posX >= XY_LENGTH-1 ? XY_LENGTH - 2 : posX;
		int posY = (int)(fishes[i].position.y + XY_LENGTH / 2);
		posY = posY < 0 ? 0 : posY >= XY_LENGTH-1 ? XY_LENGTH - 2 : posY;
		int posZ = (int)(fishes[i].position.z + Z_LENGTH / 2);
		posZ = posZ < 0 ? 0 : posZ >= Z_LENGTH-1 ? Z_LENGTH - 2 : posZ;
		//A positive field value means that the fish is inside a wall.
		if (field(vec3(posX, posY, posZ)) > -obstacle_radius) {
			float val= field(vec3(posX, posY, posZ));
			//std::cout << val << std::endl;

			vec3 grad = cgp::normalize(vec3{field(vec3(posX+1,posY,posZ))-val,field(vec3(posX,posY+1,posZ))-val,field(vec3(posX,posY,posZ+1))-val});
			fishes[i].direction-= grad/(-val)*obstacle_coef;
		}
		cgp::vec3 separation = calculate_separation(i);
		cgp::vec3 alignement = calculate_alignement(i);
		cgp::vec3 cohesion = calculate_cohesion(i);
		//std::cout << "CO:"<<cohesion << std::endl;
		//std::cout << "AL:" << alignement << std::endl;
		//std::cout << "SE:" << separation << std::endl;
		cgp::vec3 out_of_bound_force = calculate_out_of_bound_force(i);
		fishes[i].direction += separation;
		fishes[i].direction += alignement;
		fishes[i].direction += cohesion;
		fishes[i].direction += out_of_bound_force;
		if(counter%10==0)
			fishes[i].direction += {0.05 * (distrib(gen)-0.5), 0.05 * (distrib(gen) - 0.5), 0.05 * (distrib(gen) - 0.5)};
		fishes[i].direction.z *= 0.99f;
		fishes[i].direction = cgp::normalize(fishes[i].direction);
		
		fishes[i].position = fishes[i].position + (fish_speed * fishes[i].direction);

		//for (int j = 0;j < 3;j++) {
		//	fishes[i].position[j] = fishes[i].position[j] < 5 ? fishes[i].position[j] > 0 ? fishes[i].position[j] : fishes[i].position[j] + 5 : fishes[i].position[j] - 5;
		//}
	}
}

cgp::vec3 fish_manager::calculate_separation(int i) {
	
	cgp::vec3  sum = { 0,0,0 };
	int count = 0;
	for (int j = 0;j < fishes.size();j++)
	{
		cgp::vec3 delta = fishes[j].position - fishes[i].position;
		if (cgp::norm(delta) >= 0.00001 && cgp::norm(delta) < fish_radius) {
			sum += -delta / std::pow(cgp::norm(delta), 2);
			count++;
		}


	}
	if (count == 0)
		return { 0,0,0 };
	vec3 separation = separation_coef * (sum / count);
	return separation - cgp::dot(separation,fishes[i].direction)* fishes[i].direction;
}
cgp::vec3 fish_manager::calculate_alignement(int i) {
	cgp::vec3  sum = { 0,0,0 };
	int modelId = fishes[i].modelId;
	int count = 0;
	for (int j = 0;j < fishes.size();j++)
	{
		if (fishes[j].modelId == modelId) {
			cgp::vec3 delta = fishes[j].direction - fishes[i].direction;
			if (cgp::norm(delta) >= 0.001 && cgp::norm(delta) < fish_radius) {
				sum += fishes[j].direction;
				count++;
			}
		}

	}
	if (count == 0)
		return { 0,0,0 };
	return alignement_coef * (sum / count);
}


cgp::vec3 fish_manager::calculate_cohesion(int i) {
	cgp::vec3  middle = { 0,0,0 };
	int modelId = fishes[i].modelId;
	int count = 0;
	for (int j = 0;j < fishes.size();j++)
	{
		if (fishes[j].modelId == modelId) {
			cgp::vec3 delta = fishes[j].position - fishes[i].position;
			if (cgp::norm(delta) < fish_radius) {
				middle += fishes[j].position;
				count++;
			}
		}

	}

	return cohesion_coef * ((middle / count) - fishes[i].position);
}

cgp::vec3 fish_manager::calculate_out_of_bound_force(int i) {
	vec3 position = fishes[i].position;
	float out_of_bound_force = 0.04f;
	float forceX = position.x > XY_LENGTH/2 ? -out_of_bound_force : position.x < -XY_LENGTH / 2 ? out_of_bound_force : 0;
	float forceY = position.y > XY_LENGTH / 2 ? -out_of_bound_force : position.y < -XY_LENGTH / 2 ? out_of_bound_force : 0;
	float forceZ = position.z > Z_LENGTH/2 ? -out_of_bound_force : position.z < -Z_LENGTH/2 ? out_of_bound_force : 0;
	return { forceX,forceY,forceZ};
}
