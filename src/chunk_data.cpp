#include "chunk_data.hpp"


#include "cgp/cgp.hpp"

const float chunk_data::SCALE = 0.1f;

void chunk_data::initialize() {
	chunk_mesh.fill_empty_field();

	drawable.initialize_data_on_gpu(chunk_mesh);
	//drawable.model.scaling = 1.0f;
	drawable.material.color = { 0.91f, 0.6f, 0.17f };
	drawable.material.texture_settings.two_sided = true;
}
