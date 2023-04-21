

#include "scene.hpp"


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
    mesh terrain_mesh = terrain_gen.generate_terrain_mesh();
    terrain_drawable.initialize_data_on_gpu(terrain_mesh);
	terrain_drawable.model.scaling = 1.0f;
	terrain_drawable.material.color = { 0.91f, 0.6f, 0.17f };
	terrain_drawable.material.texture_settings.two_sided = true;

	// Remove warnings for unset uniforms
	cgp_warning::max_warning = 0;

	cgp::mesh test;

	// Load the custom shader
	opengl_shader_structure shader_custom;
	shader_custom.load(
		project::path + "shaders/shading_custom/vert.glsl",
		project::path + "shaders/shading_custom/frag.glsl");

	// Affect the loaded shader to the mesh_drawable
	terrain_drawable.shader = shader_custom;
}

float const MOVE_SPEED = .1f;

// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{
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
    draw(terrain_drawable, environment);
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

	/*
	bool update = false;
	update |= ImGui::SliderFloat("Persistance", &parameters.persistency, 0.1f, 0.6f);
	update |= ImGui::SliderFloat("Frequency gain", &parameters.frequency_gain, 1.5f, 2.5f);
	update |= ImGui::SliderInt("Octave", &parameters.octave, 1, 8);
	update |= ImGui::SliderFloat("Multiplier", &parameters.multiplier, 0.1f, 1.5f);

	// Update terrain if needed
	if (update) {
		mesh terrain_mesh = terrain_gen.generate_terrain_mesh(parameters);
		terrain_drawable.initialize_data_on_gpu(terrain_mesh);
	}*/
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

