#include "scene.hpp"

using namespace cgp;

// This function is called only once at the beginning of the program
// This function can contain any complex operation that can be pre-computed once
void scene_structure::initialize()
{
	// Initialize time
	timer.start();
	timer.scale = 1.5f;
	particles.initialize(timer.t, project::path);
	
	// Initialize random
	std::random_device random_device;
	rand_gen = std::mt19937(random_device());
	rand_double = std::uniform_real_distribution<>(0.0f, 1.0f);

	// Set the behavior of the camera and its initial position
	// ********************************************** //
	camera_control.initialize(inputs, window); 
	camera_control.set_rotation_axis_z(); // camera rotates around z-axis
	// look_at(camera_position, targeted_point, up_direction)
	camera_control.camera_model.distance_to_center = 0.0f;

	// Load skybox
	// ***************************************** //
	image_structure image_skybox_template = image_load_file("assets/skybox/skybox_01.jpg"); // hdr_01.png OR skybox_01.jpg
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

	test_drawable.initialize_data_on_gpu(mesh_primitive_sphere(3.0f));

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
	implicit_surface.drawable_param.shape.material.texture_settings.use_normal_map = true;
	implicit_surface.drawable_param.shape.material.phong.ambient = .3f;
	implicit_surface.drawable_param.shape.material.phong.diffuse = .8f;
	implicit_surface.drawable_param.shape.material.phong.specular = .03f;
	implicit_surface.drawable_param.shape.material.phong.specular_exponent = 2;
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
	fish_manager.initialize(environment.domain.length, environment.floor_level, project::path);

	for (int i = 0;i < fish_manager.fish_groups_number;i++) {

		// Group properties
		int const fish_type = std::rand() % 5;
		cgp::mesh_drawable const fish_model = fish_manager.fish_models.at(fish_type);
		vec3 const group_dir = 2 * vec3(rand_double(rand_gen) - .5f, rand_double(rand_gen) - .5f, rand_double(rand_gen) - .5f);
		vec3 group_pos;
		do {
			group_pos = { environment.domain.length.x * (rand_double(rand_gen) - 0.5), environment.domain.length.y * (rand_double(rand_gen) - 0.5), environment.floor_level * (.5f)};
		} while (field_function(group_pos) <= 0);

		// Spawn fishes of group
		for (int j = 0; j < fish_manager.fishes_per_group; j++) {
			fish fish;
			fish.speed = fish_manager.fish_speed;
			fish.frequency = 12.0f + 6.0f * rand_double(rand_gen);
			fish.position = group_pos + 20.0f * vec3(rand_double(rand_gen) - .5f, rand_double(rand_gen) - .5f, rand_double(rand_gen) - .5f);
			fish.direction = group_dir;
			fish.modelId = fish_type;
			fish.model = fish_model;
			fish_manager.fishes.push_back(fish);
		}
	}

	// Spawn algas
	terrain.initialize(project::path);
	for (int j = 0;j < terrain.num_group;j++) {
		float const x = environment.domain.length.x * (rand_double(rand_gen) - 0.5f);
		float const y = environment.domain.length.y * (rand_double(rand_gen) - 0.5f);
		vec3 const group_position = { x, y, get_height(x, y) };
		int const number_group_algas = std::rand() % (terrain.max_alga_per_group - terrain.min_alga_per_group) + terrain.min_alga_per_group;

		std::vector<alga> algas;
		for (int i = 0; i < number_group_algas;i++) {
			struct alga alga;
			alga.position = group_position + 30.0f * vec3{ 5 * rand_double(rand_gen) - 2.5f, 2 * rand_double(rand_gen) - 2.5f, 0 };
			alga.amplitude = 0.5 + 0.3 * rand_double(rand_gen);
			alga.frequency = 8 + 3 * rand_double(rand_gen);
			alga.rotation = rand_double(rand_gen) * 2 * std::_Pi;
			alga.scale = 1.0f + 2 * (rand_double(rand_gen) - .5f) * .3f;
			algas.push_back(alga);
		}
		struct alga_group group;
		group.algas = algas;
		terrain.alga_groups.push_back(group);
	}
	
	// Remove warnings for unset uniforms
	cgp_warning::max_warning = 0;
}

float const MOVE_SPEED = 3.0f;


static int counter = 0;

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

	// Physical model for fishes
	// ***************************************** //
	if (fish_manager.fish_groups_number > 0)
		fish_manager.refresh(field_function, timer.t);

	// Draw fishes
	// ***************************************** //
	for (int i = 0;i < fish_manager.fishes.size();i++) {
		fish fish = fish_manager.fishes[i];
		
		rotation_transform horiz_transformation = cgp::rotation_transform::from_axis_angle({ 0,0,1 }, 3.14159f / 2.0f);
		rotation_transform X_transformation = cgp::rotation_transform::from_axis_angle({ 1,0,0 }, 3.14159f / 2.0f);
		double r= sqrt(fish.direction.x * fish.direction.x +fish.direction.y*fish.direction.y+ fish.direction.z * fish.direction.z);
		double theta = acos(fish.direction.z / r);
		double psi=atan(fish.direction.y/fish.direction.x);
		if(fish.direction.x < 0)
			psi += 3.14159;
		rotation_transform Y_transformation = cgp::rotation_transform::from_axis_angle({ 0,1,0 }, theta- 3.14159f / 2.0f );
		rotation_transform Z_transformation = cgp::rotation_transform::from_axis_angle({ 0,0,1 }, psi);
		//boid.model.rotation = cgp::rotation_transform::from_vector_transform({ 0,0,1 }, boid_direction[i])*;
		//fish.model.model.rotation = cgp::rotation_transform::from_axis_angle(fish.direction, 3.14159 / 2) * cgp::rotation_transform::from_vector_transform({ 0,0,1 }, fish.direction);
		//fish.model.model.rotation = cgp::rotation_transform::from_vector_transform({ 0,0,1 }, fish.direction);
		fish.model.model.rotation = Z_transformation * Y_transformation *horiz_transformation * X_transformation;
		fish.model.model.translation = fish.position;
		//boid.material.color = boid_color[i];
		environment.uniform_generic.uniform_vec3["head_position"] = fish.position;
		environment.uniform_generic.uniform_vec3["direction"] = fish.direction;
		environment.uniform_generic.uniform_float["frequency"] = fish.frequency;
		draw(fish.model, environment);
	}
	
	// Draw algas
	// ***************************************** //
	for (alga_group group : terrain.alga_groups) {
		int counter = 0;
		for (alga alga : group.algas) {
			float flow_angle = 2 * std::_Pi * cgp::noise_perlin({ 0.01f * timer.t, 0.01f * ++counter });
			environment.uniform_generic.uniform_vec2["flow_dir"] = { cos(flow_angle), sin(flow_angle) };
			vec3 const vertical_offset = vec3{ 0.0f, 0.0f, 35.0f };
			terrain.alga_model.model.translation = alga.position + vertical_offset * alga.scale;
			terrain.alga_model.model.scaling = terrain_structure::DEFAULT_ALGA_SCALE * alga.scale;
			environment.uniform_generic.uniform_float["amplitude"] = alga.amplitude;
			environment.uniform_generic.uniform_float["frequency"] = alga.frequency;
			environment.uniform_generic.uniform_float["rotation"] = alga.rotation;
			draw(terrain.alga_model, environment);
		}
	}

	// Update time
	// ***************************************** //
	timer.update();

	// Update and draw particles
	// ***************************************** //

	if ((counter = (counter + 1) % 3) == 0) {
		vec3  random_dir = 10.0f * normalize(vec3(rand_double(rand_gen) - .5f, rand_double(rand_gen) - .5f, (rand_double(rand_gen) - .5f) / 10.0f));
		particles.register_particle(particle(vec3(0, 0, -100.0f), random_dir, 0.0f, 0.0f, 1.0f, 1.0f, .01f * vec3(1, 1, 1), .01f, 3.0f, 0));
	}

	particles.tick(timer.t);
	for (particle& particle : particles.active_particles) {
		test_drawable.model.translation = particle.position;
		draw(test_drawable, environment);
	}

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

float scene_structure::get_height(float x, float y) {
	float step = 0.1f;
	float z = 0.0f;
	while (field_function(vec3{ x, y, z }) <= environment.isovalue) {
		z -= step;
	}
	return z;
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

