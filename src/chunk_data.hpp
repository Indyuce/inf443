#pragma once

#include "cgp/cgp.hpp"

class chunk_data
{
	public:
		cgp::mesh_drawable drawable;
		cgp::grid_3D<float> grid;

		void initialize(cgp::mesh chunk_mesh,cgp::grid_3D<float> grid, cgp::opengl_shader_structure& shader);

		~chunk_data();
};

