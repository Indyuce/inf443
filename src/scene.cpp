#include "scene.hpp"

#include "terrain.hpp"
#include "tree.hpp"

using namespace cgp;

const float TERRAIN_LENGTH = 20.0f;
const int TERRAIN_SAMPLES = 100;

void scene_structure::initialize()
{
	// ============================================================================
	// INITIALIZE CAMERA
	// ============================================================================
	camera_control.initialize(inputs, window); // Give access to the inputs and window global state to the camera controler
	camera_control.set_rotation_axis_z();
	camera_control.camera_model.distance_to_center = 0;
	camera_control.camera_model.manipulator_translate_front(-5);
	camera_control.camera_model.manipulator_translate_in_plane(vec2(-5, -5));

	global_frame.initialize_data_on_gpu(mesh_primitive_frame());

	terrain_mesh = create_terrain_mesh(TERRAIN_SAMPLES, TERRAIN_LENGTH);
	terrain.initialize_data_on_gpu(terrain_mesh);
	terrain.material.phong.specular = 0.0f; // non-specular terrain material
	terrain.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/texture_grass.jpg",
		GL_MIRRORED_REPEAT,
		GL_MIRRORED_REPEAT);

	camera_sphere.initialize_data_on_gpu(mesh_primitive_sphere(30.0f));
	camera_sphere.material.color = vec3(0, 0, 0);
	camera_sphere.material.phong.ambient = 1;
	camera_sphere.material.phong.diffuse = 0;
	camera_sphere.material.phong.specular = 0;

	// Modèle arbre
	mesh tree_mesh = create_tree();
	tree.initialize_data_on_gpu(tree_mesh);

	// Initialize light
	sphere_light.initialize_data_on_gpu(mesh_primitive_sphere(.3f));

	// ============================================================================
	// CUSTOM SHADERS
	// ============================================================================

	// Light shaders

	// Load tree shader
	opengl_shader_structure shader_tree;
	shader_tree.load(
		project::path + "shaders/mesh_tree/vert.glsl",
		project::path + "shaders/mesh/frag.glsl");
	tree.shader = shader_tree;
	
	treePositions = generate_positions_on_terrain(50, TERRAIN_LENGTH, parameters);

	// Update terrain at beginning.
	update_terrain(terrain_mesh, terrain, parameters);
}

void scene_structure::display_frame()
{
	float MOVE_SPEED = .1f;

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

	// Update time
	timer.update();
	environment.uniform_generic.uniform_float["time"] = timer.t;

	// Set additional uniform parameters to the shader
	environment.uniform_generic.uniform_float["ambiant"] = light_parameters.ambiant;
	environment.uniform_generic.uniform_float["diffuse"] = light_parameters.diffuse;
	environment.uniform_generic.uniform_float["specular"] = light_parameters.specular;
	environment.uniform_generic.uniform_float["direct"] = light_parameters.direct;
	environment.uniform_generic.uniform_int["directExp"] = light_parameters.directExp;
	environment.uniform_generic.uniform_vec3["light_color"] = light_parameters.light_color;

	environment.uniform_generic.uniform_vec3["light_position"] = light_parameters.light_position;
	environment.uniform_generic.uniform_int["specularExp"] = light_parameters.specularExp;
	environment.uniform_generic.uniform_vec3["fog_color"] = environment.background_color;
	environment.uniform_generic.uniform_float["fog_distance"] = light_parameters.fog_distance;
	environment.uniform_generic.uniform_float["attenuation_distance"] = light_parameters.attenuation_distance;

	environment.uniform_generic.uniform_float["depth_buffer"] = gui.depth_buffer;

	// Update camera FoV
	//camera_sphere.model.translation = get_camera_position();
	draw(camera_sphere, environment);

	// Update light sphere
	sphere_light.model.translation = light_parameters.light_position;
	sphere_light.material.color = light_parameters.light_color;
	draw(sphere_light, environment);
	environment.uniform_generic.uniform_float["light_z_buffer"] = calculate_light_z_buffer();

	// Afficher frame
	if (gui.display_frame)
		draw(global_frame, environment);

	// Afficher terrain
	draw(terrain, environment);
	if (gui.display_wireframe)
		draw_wireframe(terrain, environment);

	// Afficher arbres
	for (int i = 0; i < treePositions.size(); i++) {
		treePositions[i].z = evaluate_terrain_height(treePositions[i].x, treePositions[i].y, TERRAIN_LENGTH, parameters);
		tree.model.translation = treePositions[i];
		draw(tree, environment);
	}
}

vec3 scene_structure::get_camera_position() {
	mat3 O = transpose(mat3(environment.camera_view)); // Get the orientation matrix
	vec4 prod = environment.camera_view * vec4(0.0, 0.0, 0.0, 1.0);
	vec3 last_col = vec3(prod.x, prod.y, prod.z);
	vec3 camera_position = -O * last_col; // Compute the position of the center of the camera
	return camera_position;
}

/*
 * This manually runs the vertex shader for the light
 * so that the GPU can know if the sun is visible or not
 */
float scene_structure::calculate_light_z_buffer() {
	return 100;
}


void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);

	ImGui::Checkbox("Depth Buffer", &gui.depth_buffer);

	bool update = false;
	update |= ImGui::SliderFloat("Persistance", &parameters.persistency, 0.1f, 0.6f);
	update |= ImGui::SliderFloat("Frequency gain", &parameters.frequency_gain, 1.5f, 2.5f);
	update |= ImGui::SliderInt("Octave", &parameters.octave, 1, 8);
	update |= ImGui::SliderFloat("Height", &parameters.terrain_height, 0.1f, 1.5f);

	update |= ImGui::ColorEdit3("Light color", &light_parameters.light_color[0]);
	update |= ImGui::SliderFloat3("Light position", &light_parameters.light_position[0], -3.0f, 3.0f);

	update |= ImGui::SliderFloat("Ambiant", &light_parameters.ambiant, 0.0f, 1.0f);
	update |= ImGui::SliderFloat("Diffuse", &light_parameters.diffuse, 0.0f, 1.0f);
	update |= ImGui::SliderFloat("Specular", &light_parameters.specular, 0.0f, 1.0f);
	update |= ImGui::SliderInt("Specular Exp", &light_parameters.specularExp, 1, 255);

	update |= ImGui::SliderFloat("Direct", &light_parameters.direct, 0.0f, 10.0f);
	update |= ImGui::SliderInt("Direct Exp", &light_parameters.directExp, 1, 255);

	update |= ImGui::ColorEdit3("Fog Color", &environment.background_color[0]);
	update |= ImGui::SliderFloat("Fog Distance", &light_parameters.fog_distance, 0.0f, 100.0f);
	update |= ImGui::SliderFloat("Attenuation Distance", &light_parameters.attenuation_distance, 0.0f, 100.0f);

	if (update)// if any slider has been changed - then update the terrain
		update_terrain(terrain_mesh, terrain, parameters);
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

	// menu key
	if (camera_control.inputs->keyboard.is_pressed(GLFW_KEY_E)) {
		camera_control.in_menu = !camera_control.in_menu;
	}

	camera_control.action_keyboard(environment.camera_view);
}
void scene_structure::idle_frame()
{
	camera_control.idle_frame(environment.camera_view);
}

