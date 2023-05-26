

#include "scene.hpp"
#include "animation.hpp"
#include <random>

using namespace cgp;

scene_structure::scene_structure() {
	num_fishes = 0;
	dt = 0.05;
	t = 0;
}

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

	// Load terrain and water shader
	// ***************************************** //
	environment.shader.load(
		project::path + "shaders/shading_custom/vert.glsl",
		project::path + "shaders/shading_custom/frag.glsl");

	// Load skybox
	// ***************************************** //
	image_structure image_skybox_template = image_load_file("assets/skybox/skybox_05.png");
	std::vector<image_structure> image_grid = image_split_grid(image_skybox_template, 4, 3);
	skybox.initialize_data_on_gpu();
	skybox.texture.initialize_cubemap_on_gpu(
		image_grid[1].mirror_vertical().rotate_90_degrees_counterclockwise(),
		image_grid[7].mirror_vertical().rotate_90_degrees_clockwise(),
		image_grid[10].mirror_horizontal(),
		image_grid[4].mirror_vertical(),
		image_grid[5],
		image_grid[3].mirror_vertical()
	);
	skybox.shader.load(
		project::path + "shaders/skybox/vert.glsl",
		project::path + "shaders/skybox/frag.glsl");

	// Initialization for the Terrain (Implicit Surface)
	// ***************************************** //
	implicit_surface.set_shader(&environment.shader);
	implicit_surface.set_domain(environment.domain.samples, environment.domain.length);
	implicit_surface.update_field(field_function, environment.isovalue);

    // Terrain
	// ***************************************** //
	//drawable_chunk = terrain_gen.generate_chunk_data(0, 0, shader_custom);

	// Animation and models
	// ***************************************** //
	initialize_models();
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

float const MOVE_SPEED = .5f;

/// <summary>
/// Angle of light relative to horizon. Light is not modeled by a position
/// but rather by a direction as light from the sun is supposed to be infinitely
/// far away. This means all light rays have approximately the same direction.
/// Shaders directly use light direction instead of position.
/// </summary>
float const LIGHT_ANGLE = 60;

vec3 scene_structure::get_camera_position() {
	const mat3 O = transpose(mat3(environment.camera_view)); // get the orientation matrix
	const vec4 last_col_4 = environment.camera_view * vec4(0.0, 0.0, 0.0, 1.0); // get the last column
	const vec3 last_col = vec3(last_col_4.x, last_col_4.y, last_col_4.z);
	return -O * last_col;
}

vec3 compute_direction(const float& azimut, const float& polar) {
	const float azimut_rad = azimut * Pi / 180.0f;
	const float polar_rad = polar * Pi / 180.0f;
	return vec3(cos(azimut_rad) * sin(polar_rad), sin(azimut_rad) * sin(polar_rad), cos(polar_rad));
}

vec3 compute_direction(vec2& dir) {
	return compute_direction(dir.x, dir.y);
}

// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{

	// Draw skybox
	//  Must be called before drawing the other shapes and without writing in the Depth Buffer
	glDepthMask(GL_FALSE); // disable depth-buffer writing
	draw(skybox, environment);
	glDepthMask(GL_TRUE);  // re-activate depth-buffer write

	// Increment time
	t += dt;
	if (num_fishes > 0)
		fish_manager.refresh(field_function);

	// Draw fishes
	for (int i = 0;i < fish_manager.fishes.size();i++) {
		fish fish = fish_manager.fishes[i];
		
		rotation_transform horiz_transformation = cgp::rotation_transform::from_axis_angle({ 0,0,1 }, 3.14159 / 2);
		rotation_transform X_transformation = cgp::rotation_transform::from_axis_angle({ 1,0,0 }, 3.14159 / 2);
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
	else if (camera_control.inputs->keyboard.shift)
		camera_control.camera_model.manipulator_translate_in_plane(vec2(0, MOVE_SPEED));

	// Send uniforms
	environment.uniform_generic.uniform_float["ambiant"] = environment.ambiant;
	environment.uniform_generic.uniform_float["diffuse"] = environment.diffuse;
	environment.uniform_generic.uniform_float["specular"] = environment.specular;
	environment.uniform_generic.uniform_float["direct"] = environment.direct;
	environment.uniform_generic.uniform_int["direct_exp"] = environment.direct_exp;
	environment.uniform_generic.uniform_vec3["light_color"] = environment.light_color;

	environment.uniform_generic.uniform_float["flashlight"] = environment.flashlight;
	environment.uniform_generic.uniform_int["flashlight_exp"] = environment.flashlight_exp;

	environment.uniform_generic.uniform_vec3["light_direction"] = compute_direction(environment.light_direction);
	environment.uniform_generic.uniform_int["specularExp"] = environment.specular_exp;
	environment.uniform_generic.uniform_vec3["fog_color"] = environment.background_color;
	environment.uniform_generic.uniform_float["fog_distance"] = environment.fog_distance;
	// environment.uniform_generic.uniform_float["attenuation_distance"] = environment.attenuation_distance;
	environment.uniform_generic.uniform_float["time"] = t;

	// Draw terrain
	draw(implicit_surface.drawable_param.domain_box, environment);
	draw(implicit_surface.drawable_param.shape, environment);
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

		ImGui::SliderFloat("Flashlight", &environment.flashlight, 0.0f, 10.0f);
		ImGui::SliderInt("Flashlight Exp", &environment.flashlight_exp, 1, 255);
		// ImGui::SliderFloat("Flashlight Dist", &environment.flashlight_dist, 1.0f, 100.0f);

		// Now Hard coded
		ImGui::SliderFloat("Direct", &environment.direct, 0.0f, 10.0f);
		ImGui::SliderInt("Direct Exp", &environment.direct_exp, 1, 1000);

		ImGui::ColorEdit3("Fog Color", &environment.background_color[0]);
		ImGui::SliderFloat("Fog Distance", &environment.fog_distance, 100.0f, 1000.0f);
		// ImGui::SliderFloat("Attenuation Distance", &environment.attenuation_distance, 100.0f, 1000.0f);
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

