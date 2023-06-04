#include "terrain.hpp"
#include <random>

using namespace cgp;

float const terrain_structure::DEFAULT_ALGA_SCALE = 40.0f;

void terrain_structure::initialize(std::string project_path)
{
	this->num_group = 10;
	this->min_alga_per_group = 15;
	this->max_alga_per_group = 30;

	alga_model.initialize_data_on_gpu(mesh_load_file_obj(project_path + "assets/alga/alga.obj"));
	alga_model.texture.load_and_initialize_texture_2d_on_gpu(project_path + "assets/alga/alga.jpeg");
	opengl_shader_structure alga_shader;
	alga_shader.load(
		project_path + "shaders/alga/vert.glsl",
		project_path + "shaders/terrain/frag.glsl");
	alga_model.shader = alga_shader;
	alga_model.material.phong = { 1.0f, 0, 0,0 };
}
