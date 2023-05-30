

#include "scene.hpp"
#include <random>

using namespace cgp;

scene_structure::scene_structure() {
	num_fishes = 0;
}

// This function is called only once at the beginning of the program
// This function can contain any complex operation that can be pre-computed once
void scene_structure::initialize()
{
	timer.start();
	timer.scale = 2.0f;

	// Set the behavior of the camera and its initial position
	// ********************************************** //
	camera_control.initialize(inputs, window); 
	camera_control.set_rotation_axis_z(); // camera rotates around z-axis
	// look_at(camera_position, targeted_point, up_direction)
	camera_control.camera_model.distance_to_center = 0.0f;

	// Load skybox
	// ***************************************** //
	image_structure image_skybox_template = image_load_file("assets/skybox/hdr_01.png"); // hdr_01.png OR skybox_01.jpg
	std::vector<image_structure> image_grid = image_split_grid(image_skybox_template, 4, 3);
	skybox.initialize_data_on_gpu();
	skybox.texture.initialize_cubemap_on_gpu(
		image_grid[1].mirror_vertical().rotate_90_degrees_counterclockwise(),
		image_grid[7].mirror_vertical().rotate_90_degrees_clockwise(),
		image_grid[10].mirror_horizontal(),
		image_grid[4].mirror_vertical(),
		image_grid[5].mirror_horizontal(),
		image_grid[3].mirror_vertical()
	);
	skybox.shader.load(
		project::path + "shaders/skybox/vert.glsl",
		project::path + "shaders/skybox/frag.glsl");

	// Load terrain + shader
	// ***************************************** //
	environment.shader.load(
		project::path + "shaders/terrain/vert.glsl",
		project::path + "shaders/terrain/frag.glsl");

	field_function.floor_level = environment.floor_level;
	implicit_surface.floor_level = environment.floor_level;
	implicit_surface.shader = environment.shader;
	implicit_surface.set_domain(environment.domain.resolution, environment.domain.length);
	implicit_surface.update_field(field_function, environment.isovalue);
	implicit_surface.drawable_param.shape.texture.load_and_initialize_texture_2d_on_gpu(
		project::path + "assets/texture/cartoon_sand/Basecolor.png",
		GL_REPEAT,
		GL_REPEAT);
	implicit_surface.drawable_param.shape.supplementary_texture["normal_map"].load_and_initialize_texture_2d_on_gpu(
		project::path + "assets/texture/cartoon_sand/Base_Normal.png",
		GL_REPEAT,
		GL_REPEAT);
	implicit_surface.drawable_param.shape.supplementary_texture["height_map"].load_and_initialize_texture_2d_on_gpu(
		project::path + "assets/texture/cartoon_sand/Base_height.png",
		GL_REPEAT,
		GL_REPEAT);
	//drawable_chunk = terrain_gen.generate_chunk_data(0, 0, shader_custom);

	// Load water surface & shader
	// ***************************************** //
	water_surface.initialize_models();
	opengl_shader_structure water_shader;
	water_shader.load(
		project::path + "shaders/water_surface/vert.glsl",
		project::path + "shaders/water_surface/frag.glsl");
	water_surface.set_shaders(water_shader);
	water_surface.set_textures(implicit_surface.drawable_param.shape.texture, skybox.texture);

	// Animation and models
	// ***************************************** //
	//initialize_models();
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distrib(0, 1);
	
	//boid.initialize_data_on_gpu(create_cone_mesh(0.05, 0.1, 0));
	
	fish fish;
	for (int i = 0;i < num_fishes;i++) {
		fish.direction = { 2*distrib(gen)-1,2*distrib(gen)-1,2*distrib(gen)-1 };
		do{
			fish.position = { XY_LENGTH * (distrib(gen) - 0.5), XY_LENGTH * (distrib(gen) - 0.5), Z_LENGTH * (distrib(gen) - 0.5) };
		} while (field_function(vec3(fish.position.x, fish.position.y, fish.position.z)) < -1);

		fish.speed = fish_manager.fish_speed;
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
}

float const MOVE_SPEED = 3.0f;

// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{

	// Draw skybox
	// Must be called before drawing the other shapes and without writing in the Depth Buffer
	// ***************************************** //
	glDepthMask(GL_FALSE); // disable depth-buffer writing
	draw(skybox, environment);
	glDepthMask(GL_TRUE);  // re-activate depth-buffer write

	// Increment time
	// ***************************************** //
	if (num_fishes > 0)
		fish_manager.refresh(field_function);

	// Draw fishes
	// ***************************************** //
	for (int i = 0;i < fish_manager.fishes.size();i++) {
		fish fish = fish_manager.fishes[i];
		
		rotation_transform horiz_transformation = cgp::rotation_transform::from_axis_angle({ 0,0,1 }, 3.14159f / 2.0f);
		rotation_transform X_transformation = cgp::rotation_transform::from_axis_angle({ 1,0,0 }, 3.14159f / 2.0f);
		//boid.model.rotation = cgp::rotation_transform::from_vector_transform({ 0,0,1 }, boid_direction[i])*;
		//fish.model.model.rotation = cgp::rotation_transform::from_axis_angle(fish.direction, 3.14159 / 2) * cgp::rotation_transform::from_vector_transform({ 0,0,1 }, fish.direction);
		fish.model.model.rotation = cgp::rotation_transform::from_vector_transform({ 0,0,1 }, fish.direction);
		fish.model.model.translation = fish.position;
		fish.model.material.color = { 1,1,1 };
		//boid.material.color = boid_color[i];
		environment.uniform_generic.uniform_vec3["head_position"] = fish.position;
		environment.uniform_generic.uniform_vec3["direction"] = fish.direction;
		environment.uniform_generic.uniform_float["frequency"] = fish.frequency;
		draw(fish.model, environment);
	}

	// Update time
	// ***************************************** //
	timer.update();

	// Move player
	// ***************************************** //
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
	else if (camera_control.inputs->keyboard.shift)
		camera_control.camera_model.manipulator_translate_in_plane(vec2(0, MOVE_SPEED));

	// Uniforms
	// ***************************************** //

	// Time
	environment.uniform_generic.uniform_float["time"] = timer.t;

	// Get camera location
	vec3 const camera_position = environment.get_camera_position();
	environment.uniform_generic.uniform_vec3["camera_position"] = camera_position;
	environment.uniform_generic.uniform_float["water_surface_plane_length"] = water_surface.total_length;
	environment.uniform_generic.uniform_float["water_surface_center_length"] = water_surface.center_length;

	// Get camera direction
	vec3 camera_direction = vec3(environment.camera_view(2, 0), environment.camera_view(2, 1), environment.camera_view(2, 2));
	environment.uniform_generic.uniform_vec3["camera_direction"] = camera_direction;

	// Draw terrain
	// ***************************************** //
	draw(implicit_surface.drawable_param.domain_box, environment);
	draw(implicit_surface.drawable_param.shape, environment);

	// Draw water surface
	// ***************************************** //
	water_surface.update_positions(camera_position);
	draw(water_surface.center, environment);
	draw(water_surface.positive_x, environment);
	draw(water_surface.negative_x, environment);
	draw(water_surface.positive_y, environment);
	draw(water_surface.negative_y, environment);
}

void scene_structure::display_gui()
{

	// Handle the gui values and the updates using the helper methods (*)
	implicit_surface.gui_update(environment, field_function);

	if (ImGui::CollapsingHeader("Environment")) {

		ImGui::ColorEdit3("Light Color", &environment.light_color[0]);
		ImGui::SliderFloat2("Light Azimut/Polar", &environment.light_direction[0], -180, 180);

		ImGui::SliderFloat("Ambiant", &environment.ambiant, 0.0f, 1.0f);
		ImGui::SliderFloat("Diffuse", &environment.diffuse, 0.0f, 1.0f);
		ImGui::SliderFloat("Specular", &environment.specular, 0.0f, 1.0f);
		ImGui::SliderInt("Specular Exp", &environment.specular_exp, 1, 255);

		ImGui::Checkbox("Toggle Flashlight", &environment.flashlight_on);
		ImGui::SliderFloat("Flashlight", &environment.flashlight, 0.0f, 10.0f);
		ImGui::SliderInt("Flashlight Exp", &environment.flashlight_exp, 1, 255);

		ImGui::SliderFloat("Direct", &environment.direct, 0.0f, 10.0f);
		ImGui::SliderInt("Direct Exp", &environment.direct_exp, 1, 1000);

		ImGui::ColorEdit3("Water Color", &environment.background_color[0]);
		ImGui::SliderFloat("Fog Distance", &environment.fog_distance, 500.0f, 5000.0f);

		ImGui::Checkbox("Water Surface Height Shader", &environment.surf_height);
		ImGui::SliderFloat("Water Optical Index", &environment.water_optical_index, 0.5f, 2.0f);
		ImGui::SliderFloat("Terrain Ridges", &environment.terrain_ridges, 0.0f, 10.0f);
		ImGui::SliderFloat("Color Attenuation Scale", &environment.scale, 0.001f, 0.1f);
	}
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

	fish0.model.scaling = 0.5f;
	fish1.model.scaling = 0.5f;
	fish2.model.scaling = 1.0f;
	fish3.model.scaling = 0.5f;
	fish4.model.scaling = 4.0f;
	
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

