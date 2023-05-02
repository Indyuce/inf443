#include "animation.hpp"

using namespace cgp;

fish_manager::fish_manager() {
	this->alignement_coef = 0.001f;
	this->cohesion_coef = 0.4f;
	this->separation_coef = 0.01f;
	this->fish_radius = 1.0f;
	this->fish_speed = 0.01f;
	//, ,, , , 1, 3, 5
}

void fish_manager::add(fish fish) {
	fishes.push_back(fish);
}

void fish_manager::refresh() {

	for (int i = 0;i < fishes.size();i++) {
		cgp::vec3 separation = calculate_separation(i);
		cgp::vec3 alignement = calculate_alignement(i);
		cgp::vec3 cohesion = calculate_cohesion(i);
		fishes[i].direction += separation;
		fishes[i].direction += alignement;
		fishes[i].direction += cohesion;
		fishes[i].direction = cgp::normalize(fishes[i].direction);
		fishes[i].position = fishes[i].position + (fish_speed * fishes[i].direction);

		for (int j = 0;j < 3;j++) {
			fishes[i].position[j] = fishes[i].position[j] < 5 ? fishes[i].position[j] > 0 ? fishes[i].position[j] : fishes[i].position[j] + 5 : fishes[i].position[j] - 5;
		}
	}
}

cgp::vec3 fish_manager::calculate_separation(int i) {
	
	cgp::vec3  sum = { 0,0,0 };
	for (int j = 0;j < fishes.size();j++)
	{
		cgp::vec3 delta = fishes[j].position - fishes[i].position;
		if (cgp::norm(delta) >= 0.001 && cgp::norm(delta) < fish_radius) {
			sum += -delta / std::pow(cgp::norm(delta), 2);
		}

	}
	return separation_coef * sum;
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

	return cohesion_coef * (middle / count - fishes[i].position);
}
