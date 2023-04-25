

#include "scene.hpp"
#include "animation.cpp"
#include <random>

using namespace cgp;


// This function is called only once at the beginning of the program
// This function can contain any complex operation that can be pre-computed once
void scene_structure::initialize()
{

	// Set the behavior of the camera and its initial position
	// ********************************************** //
	camera_control.initialize(inputs, window); 
	camera_control.set_rotation_axis_z(); // camera rotates around z-axis
	//   look_at(camera_position, targeted_point, up_direction)
	camera_control.camera_model.distance_to_center = 0.0f;

	// Create the shapes seen in the 3D scene
	// ********************************************** //

    // Terrain
	drawable_chunk = terrain_gen.generate_chunk_data(0, 0);

	initialize_models();
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(0, 1);
	
	//boid.initialize_data_on_gpu(create_cone_mesh(0.05, 0.1, 0));
	boid_speed = 0.01;
	boid_radius = 1;
	separation_coef = 0.01;
	alignement_coef = 0.01;
	cohesion_coef = 0.4;

	fish_manager = fish_manager::fish_manager(0.001, 0.4, 0.01, 1,0.01,1,3,5);
	change_color_coef = 0.5;
	num_fishes = 0;
	dt = 0.05;
	t = 0;

	fish fish;
	for (int i = 0;i < num_fishes;i++) {
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
		fish_manager.add(fish);
	}

	// Remove warnings for unset uniforms
	cgp_warning::max_warning = 0;

	// Load the custom shader
	cgp::opengl_shader_structure shader_custom;
	shader_custom.load(
		project::path + "shaders/shading_custom/vert.glsl",
		project::path + "shaders/shading_custom/frag.glsl");

	// Affect the loaded shader to the mesh_drawable
	drawable_chunk->drawable.shader = shader_custom;
}

float const MOVE_SPEED = .1f;

// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{
	t += dt;
	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();
	environment.uniform_generic.uniform_float["time"] = t;
	for (int i = 0;i < fish_manager.fishes.size();i++) {
		fish fish = fish_manager.fishes[i];
		rotation_transform horiz_transformation = cgp::rotation_transform::from_axis_angle({ 0,0,1 }, 3.14159 / 2);
		rotation_transform X_transformation = cgp::rotation_transform::from_axis_angle({ 1,0,0 }, 3.14159 / 2);
		//boid.model.rotation = cgp::rotation_transform::from_vector_transform({ 0,0,1 }, boid_direction[i])*;
		fish.model.model.rotation = cgp::rotation_transform::from_axis_angle(fish.direction, 3.14159 / 2) * cgp::rotation_transform::from_vector_transform({ 0,0,1 }, fish.direction);
		fish.model.model.translation = fish.position;
		fish.model.material.color = { 1,1,1 };
		//boid.material.color = boid_color[i];
		environment.uniform_generic.uniform_vec3["head_position"] = fish.position;
		environment.uniform_generic.uniform_vec3["direction"] = fish.direction;
		environment.uniform_generic.uniform_float["frequency"] = fish.frequency;
		draw(fish.model, environment);
	}
	// Update time
	timer.update();

	// Move player
	if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_W))
		camera_control.camera_model.manipulator_translate_front(-MOVE_SPEED);
	else if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_S))
		camera_control.camera_model.manipulator_translate_front(MOVE_SPEED);

	if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_A))
		camera_control.camera_model.manipulator_translate_in_plane(vec2(MOVE_SPEED, 0));
	else if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_D))
		camera_control.camera_model.manipulator_translate_in_plane(vec2(-MOVE_SPEED, 0));

	if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_SPACE))
		camera_control.camera_model.manipulator_translate_in_plane(vec2(0, -MOVE_SPEED));
	else if (camera_control.inputs->keyboard.ctrl)
		camera_control.camera_model.manipulator_translate_in_plane(vec2(0, MOVE_SPEED));

	// Send uniforms
	environment.uniform_generic.uniform_float["ambiant"] = gui.ambiant;
	environment.uniform_generic.uniform_float["diffuse"] = gui.diffuse;
	environment.uniform_generic.uniform_float["specular"] = gui.specular;
	environment.uniform_generic.uniform_float["direct"] = gui.direct;
	environment.uniform_generic.uniform_int["directExp"] = gui.directExp;
	environment.uniform_generic.uniform_vec3["light_color"] = gui.light_color;

	environment.uniform_generic.uniform_vec3["light_position"] = gui.light_position;
	environment.uniform_generic.uniform_int["specularExp"] = gui.specularExp;
	environment.uniform_generic.uniform_vec3["fog_color"] = environment.background_color;
	environment.uniform_generic.uniform_float["fog_distance"] = gui.fog_distance;
	environment.uniform_generic.uniform_float["attenuation_distance"] = gui.attenuation_distance;

	// the general syntax to display a mesh is:
	//   draw(mesh_drawableName, environment);
	// Note: scene is used to set the uniform parameters associated to the camera, light, etc. to the shader
    draw(drawable_chunk->drawable, environment);
}

void scene_structure::display_gui()
{
	ImGui::ColorEdit3("Light color", &gui.light_color[0]);
	ImGui::SliderFloat3("Light position", &gui.light_position[0], -3.0f, 3.0f);

	ImGui::SliderFloat("Ambiant", &gui.ambiant, 0.0f, 1.0f);
	ImGui::SliderFloat("Diffuse", &gui.diffuse, 0.0f, 1.0f);
	ImGui::SliderFloat("Specular", &gui.specular, 0.0f, 1.0f);
	ImGui::SliderInt("Specular Exp", &gui.specularExp, 1, 255);

	ImGui::SliderFloat("Direct", &gui.direct, 0.0f, 10.0f);
	ImGui::SliderInt("Direct Exp", &gui.directExp, 1, 255);

	ImGui::ColorEdit3("Fog Color", &environment.background_color[0]);
	ImGui::SliderFloat("Fog Distance", &gui.fog_distance, 100.0f, 1000.0f);
	ImGui::SliderFloat("Attenuation Distance", &gui.attenuation_distance, 100.0f, 1000.0f);
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


	fish4.initialize_data_on_gpu(mesh_load_file_obj(project::path+"assets/fish4/fish4.obj"));
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


	alga.initialize_data_on_gpu(mesh_load_file_obj(project::path + "assets/alga/alga.obj"));
	alga.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/alga/alga.jpeg");
	opengl_shader_structure alga_shader;
	alga_shader.load(
		project::path + "shaders/alga/vert.glsl",
		project::path + "shaders/alga/frag.glsl");
	alga.shader = alga_shader;
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

