#pragma once

#include "cgp/cgp.hpp"

class chunk_data
{
	public:
		bool* voxel;
		cgp::mesh chunk_mesh;
		cgp::mesh_drawable drawable;

		chunk_data() {
			voxel = new bool[chunk_data::VOXEL_SIZE];
		}

		void initialize();

		static const int CHUNK_SIZE = 50;
		static const int CHUNK_HEIGHT = 100;
		static const float SCALE;

		static const int VOXEL_SIZE = CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT;
};

