#include "chunk_data.hpp"
#include "cgp/cgp.hpp"

void chunk_data::initialize(cgp::mesh chunk_mesh, cgp::grid_3D<float> grid,cgp::opengl_shader_structure& shader) {
	chunk_mesh.normal_update();
	chunk_mesh.fill_empty_field();
	this->grid = grid;
	drawable.initialize_data_on_gpu(chunk_mesh);
	//drawable.model.scaling = 1.0f;
	drawable.material.color = { 0.91f, 0.6f, 0.17f };
	drawable.shader = shader;
	//drawable.material.texture_settings.two_sided = true;
}

chunk_data::~chunk_data() {
	delete &drawable;
}
