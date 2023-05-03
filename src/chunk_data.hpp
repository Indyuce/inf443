#pragma once

#include "cgp/cgp.hpp"

class chunk_data
{
	public:
		cgp::mesh_drawable drawable;

		void initialize(cgp::mesh chunk_mesh, cgp::opengl_shader_structure& shader);

		~chunk_data();
};

