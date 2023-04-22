#include "scene.hpp"

#include "terrain.hpp"
#include <random>

using namespace cgp;




void scene_structure::initialize()
{
	camera_control.initialize(inputs, window); // Give access to the inputs and window global state to the camera controler
	camera_control.set_rotation_axis_z();
	camera_control.look_at({ 15.0f,6.0f,6.0f }, {0,0,0});;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(0, 1);
	
	
	//boid.initialize_data_on_gpu(create_cone_mesh(0.05, 0.1, 0));
	boid_speed = 0.01;
	boid_radius = 1;
	separation_coef = 0.01;
	alignement_coef = 0.01;
	cohesion_coef = 0.4;
	change_color_coef = 0.5;
	num_fishes = 0;
	dt = 0.05;
	t = 0;
	//colors = {
	//cgp::vec3(0.97f, 0.16f, 0.36f), // bright red
	//cgp::vec3(0.98f, 0.73f, 0.05f), // orange
	//cgp::vec3(0.94f, 0.76f, 0.32f), // yellow
	//cgp::vec3(0.16f, 0.50f, 0.73f), // bright blue
	//cgp::vec3(0.36f, 0.77f, 0.87f), // light blue
	//cgp::vec3(0.01f, 0.85f, 0.67f), // turquoise
	//cgp::vec3(0.12f, 0.58f, 0.48f), // teal
	//cgp::vec3(0.58f, 0.38f, 0.73f), // purple
	//cgp::vec3(0.87f, 0.36f, 0.70f), // magenta
	//cgp::vec3(0.95f, 0.65f, 0.76f), // pink
	//cgp::vec3(0.99f, 0.99f, 0.99f)  // almost white
	//};

	for (int i = 0;i < num_fishes;i++) {
		fish fish;
		fish.direction = { 1,0,0 };
		fish.position = { 5 * distrib(gen), 5 * distrib(gen), 5 * distrib(gen) };
		fish.speed = boid_speed;
		fish.frequency = 4 + 2 * distrib(gen);
		int random = std::rand()%5;
		fish.modelId = random;
		switch (random) {
		case 0:
			fish.model = fish0;
			break;
		case 1:
			fish.model = fish1;
			break;
		case 2:
			fish.model = fish2;
			break;
		case 3:
			fish.model = fish3;
			break;
		case 4:
			fish.model = fish4;
			break;
		}
		fishes.push_back(fish);
	}
}

void scene_structure::display_frame()
{	
	t += dt;
	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();
	environment.uniform_generic.uniform_float["time"] = t;
	for (int i = 0;i < num_fishes;i++) {
		cgp::vec3 separation = calculate_separation(i);
		cgp::vec3 alignement = calculate_alignement(i);
		cgp::vec3 cohesion= calculate_cohesion(i);
		fishes[i].direction += separation;
		fishes[i].direction += alignement;
		fishes[i].direction += cohesion;
		fishes[i].direction =cgp::normalize(fishes[i].direction);
		fishes[i].position = fishes[i].position + (boid_speed * fishes[i].direction);
	
		for (int j = 0;j < 3;j++) {
			fishes[i].position[j]=fishes[i].position[j] < 5 ? fishes[i].position[j] > 0 ? fishes[i].position[j] : fishes[i].position[j] + 5 : fishes[i].position[j] - 5;
		}
		
		
		rotation_transform horiz_transformation = cgp::rotation_transform::from_axis_angle({ 0,0,1 }, 3.14159 / 2);
		rotation_transform X_transformation = cgp::rotation_transform::from_axis_angle({ 1,0,0 }, 3.14159 / 2);
		//boid.model.rotation = cgp::rotation_transform::from_vector_transform({ 0,0,1 }, boid_direction[i])*;
		fishes[i].model.model.rotation = cgp::rotation_transform::from_axis_angle(fishes[i].direction, 3.14159 / 2) * cgp::rotation_transform::from_vector_transform({0,0,1}, fishes[i].direction);
		fishes[i].model.model.translation = fishes[i].position;
		fishes[i].model.material.color = { 1,1,1 };
		//boid.material.color = boid_color[i];
		environment.uniform_generic.uniform_vec3["head_position"] = fishes[i].position;
		environment.uniform_generic.uniform_vec3["direction"] = fishes[i].direction;
		environment.uniform_generic.uniform_float["frequency"] = fishes[i].frequency;
		draw(fishes[i].model, environment);
	}

}

cgp::vec3 scene_structure::calculate_separation(int i) {
	cgp::vec3  sum= {0,0,0};
	for (int j = 0;j < num_fishes;j++)
{
		cgp::vec3 delta = fishes[j].position - fishes[i].position;
		if (cgp::norm(delta) >= 0.001&& cgp::norm(delta) < boid_radius) {
			sum += -delta / std::pow(cgp::norm(delta), 2);
		}
		
	}
	return separation_coef * sum;
}
cgp::vec3 scene_structure::calculate_alignement(int i) {
	cgp::vec3  sum = { 0,0,0 };
	int modelId = fishes[i].modelId;
	int count = 0;
	for (int j = 0;j < num_fishes;j++)
	{
		if (fishes[j].modelId == modelId) {
			cgp::vec3 delta = fishes[j].direction - fishes[i].direction;
			if (cgp::norm(delta) >= 0.001 && cgp::norm(delta) < boid_radius) {
				sum += fishes[j].direction;
				count++;
			}
		}

	}
	if (count == 0)
		return { 0,0,0 };
	return alignement_coef * (sum/count);
}


cgp::vec3 scene_structure::calculate_cohesion(int i) {
	cgp::vec3  middle = { 0,0,0 };
	int modelId = fishes[i].modelId;
	int count = 0;
	for (int j = 0;j < num_fishes;j++)
	{
		if (fishes[j].modelId == modelId) {
			cgp::vec3 delta = fishes[j].position - fishes[i].position;
			if (cgp::norm(delta) < boid_radius) {
				middle += fishes[j].position;
				count++;
			}
		}

	}
	
	return cohesion_coef * (middle/count-fishes[i].position);
}


void scene_structure::initialize_models() {
	fish0.initialize_data_on_gpu(mesh_load_file_obj(project::path+"assets/fish0/fish0.obj"));
	fish0.texture.load_and_initialize_texture_2d_on_gpu(project::path+"assets/fish0/fish0.png");
	opengl_shader_structure fish0_shader;
	fish0_shader.load(
		project::path + "shaders/fish0/vert.glsl",
		project::path + "shaders/fish0/frag.glsl");
	fish0.shader = fish0_shader;

	fish1.initialize_data_on_gpu(mesh_load_file_obj(project::path+"assets/fish1/fish1.obj"));
	fish1.texture.load_and_initialize_texture_2d_on_gpu(project::path+"assets/fish1/fish1.png");
	opengl_shader_structure fish1_shader;
	fish1_shader.load(
		project::path + "shaders/fish1/vert.glsl",
		project::path + "shaders/fish1/frag.glsl");
	fish1.shader = fish1_shader;

	fish2.initialize_data_on_gpu(mesh_load_file_obj(project::path+"assets/fish2/fish2.obj"));
	fish2.texture.load_and_initialize_texture_2d_on_gpu(project::path+"assets/fish2/fish2.jpeg");
	opengl_shader_structure fish2_shader;
	fish2_shader.load(
		project::path + "shaders/fish2/vert.glsl",
		project::path + "shaders/fish2/frag.glsl");
	fish2.shader = fish2_shader;

	fish3.initialize_data_on_gpu(mesh_load_file_obj(project::path+"assets/fish3/fish3.obj"));
	fish3.texture.load_and_initialize_texture_2d_on_gpu(project::path+"assets/fish3/fish3.png");
	opengl_shader_structure fish3_shader;
	fish3_shader.load(
		project::path + "shaders/fish3/vert.glsl",
		project::path + "shaders/fish3/frag.glsl");
	fish3.shader = fish3_shader;


	fish4.initialize_data_on_gpu(mesh_load_file_obj(project::path+"assets/assets/fish4/fish4.obj"));
	fish4.texture.load_and_initialize_texture_2d_on_gpu(project::path+"assets/fish4/fish4.jpeg");
	opengl_shader_structure fish4_shader;
	fish4_shader.load(
		project::path + "shaders/fish4/vert.glsl",
		project::path + "shaders/fish4/frag.glsl");
	fish4.shader = fish4_shader;

	fish0.model.scaling = 0.05f;
	fish1.model.scaling = 0.05f;
	fish2.model.scaling = 0.1f;
	fish3.model.scaling = 0.05f;
	fish4.model.scaling = 0.4f;
	
	//boid.model.scaling = 0.1f;  //0.04F
	jellyfish.initialize_data_on_gpu(mesh_load_file_obj(project::path+"assets/jellyfish/Jellyfish_001.obj"));
	jellyfish.texture.load_and_initialize_texture_2d_on_gpu(project::path+"assets/jellyfish/Jellyfish_001_tex.png");
	opengl_shader_structure jellyfish_shader;
	jellyfish_shader.load(
		project::path + "shaders/jellyfish/vert.glsl",
		project::path + "shaders/jellyfish/frag.glsl");
	jellyfish.shader = jellyfish_shader;
}
/*

cgp::vec3 scene_structure::calculate_color(int i) {
	cgp::vec3  mean = { 0,0,0 };
	int count = 0;
	for (int j = 0;j < num_boids;j++)
	{
		cgp::vec3 delta = boid_position[j] - boid_position[i];
		if (cgp::norm(delta) < boid_radius/4) {
			mean += boid_color[j];
			count++;
		}

	}
	if (count == 0)
		return boid_color[i];
	mean = mean / count;

	cgp::vec3 prefered_color;
	//The distance is a most 2 as it is the distance between 2 vector that have coordinates between 0 and 1.
	double min_dist = 2;
	for (int j = 0;j < colors.size();j++) {
		double dist = cgp::norm(colors[j] - mean);
		if (dist < min_dist) {
			min_dist = dist;
			prefered_color = colors[j];
		}
	}
	return (1-change_color_coef)*boid_color[i]+change_color_coef*(prefered_color);
}
*/

void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);
}

void scene_structure::mouse_move_event()
{
	if (!inputs.keyboard.shift)
		camera_control.action_mouse_move(environment.camera_view);
}
void scene_structure::mouse_click_event()
{
	camera_control.action_mouse_click(environment.camera_view);
}
void scene_structure::keyboard_event()
{
	camera_control.action_keyboard(environment.camera_view);
}
void scene_structure::idle_frame()
{
	camera_control.idle_frame(environment.camera_view);
}

