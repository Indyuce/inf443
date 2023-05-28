#include "implicit_surface.hpp"


using namespace cgp;


static void update_normals(std::vector<vec3>& normals, int number_of_vertex, grid_3D<vec3> const& gradient, std::vector<marching_cube_relative_coordinates> const& relative_coords)
{
	// Compute the normal using linear interpolation of the gradients
	for (int k = 0; k < number_of_vertex; ++k)
	{
		size_t const idx0 = relative_coords[k].k0;
		size_t const idx1 = relative_coords[k].k1;

		vec3 const& n0 = gradient.at_unsafe(idx0);
		vec3 const& n1 = gradient.at_unsafe(idx1);

		float const alpha = relative_coords[k].alpha;
		normals[k] = -normalize((1 - alpha) * n0 + alpha * n1, { 1,0,0 });
	}
}

static void update_colors(std::vector<vec3>& color, int number_of_vertex, std::vector<vec3>& position, field_function_structure const& field_function) {
	for (int k = 0; k < number_of_vertex; ++k) {
		color[k] = field_function.color_at(position[k]);
	}
}

void implicit_surface_structure::update_marching_cube(field_function_structure const& field_function, float isovalue)
{
	// Variable shortcut
	std::vector<vec3>& position = data_param.position;
	std::vector<vec3>& normal = data_param.normal;
	//std::vector<vec3>& color = data_param.color;
	size_t& number_of_vertex = data_param.number_of_vertex;
	spatial_domain_grid_3D const& domain = field_param.domain;
	std::vector<cgp::marching_cube_relative_coordinates> relative_coord = data_param.relative;
	grid_3D<float> const& field = field_param.field;
	grid_3D<vec3> const& gradient = field_param.gradient;

	// Store the size of the previous position buffer
	size_t const previous_size = position.size();

	// Compute the Marching Cube
	number_of_vertex = marching_cube(position, field.data.data, domain, isovalue, &relative_coord);

	// Resize the vector of normals if needed
	if (normal.size() < position.size()) {
		normal.resize(position.size());
		//color.resize(position.size());
	}

	update_normals(normal, number_of_vertex, gradient, relative_coord);
	//update_colors(color, number_of_vertex, position, field_function);

	// Update the display of the mesh
	if (position.size() > previous_size) {
		// If there is more position than allocated - perform a full clear and reallocation from scratch
		drawable_param.shape.clear();


		std::cout << "Initializing on GPU" << std::endl;

		drawable_param.shape.initialize_data_on_gpu(position, normal);
	}
	else {
		// Otherwise simply update the new relevant values re-using the allocated buffers
		drawable_param.shape.vbo_position.update(position, number_of_vertex);
		drawable_param.shape.vbo_normal.update(normal, number_of_vertex);
		//drawable_param.shape.vbo_color.update(color, number_of_vertex);
		drawable_param.shape.vertex_number = number_of_vertex;
	}

	drawable_param.shape.shader = shader;
	drawable_param.shape.model.translation.z = floor_level;
}


void implicit_surface_structure::update_field(field_function_structure const& field_function, float isovalue)
{
	// Variable shortcut
	grid_3D<float>& field = field_param.field;
	grid_3D<vec3>& gradient = field_param.gradient;
	spatial_domain_grid_3D& domain = field_param.domain;

	// Compute the scalar field
	field = compute_discrete_scalar_field(domain, field_function);

	// Compute the gradient of the scalar field
	gradient = compute_gradient(field);

	// Recompute the marching cube
	update_marching_cube(field_function, isovalue);

	// Reset the domain visualization (lightweight - can be cleared at each call)
	drawable_param.domain_box.clear();
	drawable_param.domain_box.initialize_data_on_gpu(domain.export_segments_for_drawable_border());
	drawable_param.domain_box.model.translation.z = floor_level;
}

int3 to_int3(vec3 const& vec) {
	return int3((int)vec.x, (int)vec.y, (int)vec.z);
}

void implicit_surface_structure::set_domain(int resolution, cgp::vec3 const& length)
{
	field_param.domain = spatial_domain_grid_3D::from_center_length({ 0, 0, length.z / 2.0f + floor_level }, length, to_int3(length / resolution));
}

void implicit_surface_structure::display_gui_implicit_surface(bool& is_update_field, bool& is_update_marching_cube, bool& is_save_obj, environment_structure& gui, field_function_structure& field_function)
{
	if (ImGui::CollapsingHeader("Marching Cube"))
	{
		is_update_field |= ImGui::SliderInt("Resolution", &gui.domain.resolution, 1, 10);

		//is_update_field |= ImGui::SliderFloat("Lx", &gui.domain.length.x, 0.5f, 10.0f);
		//is_update_field |= ImGui::SliderFloat("Ly", &gui.domain.length.y, 0.5f, 10.0f);
		//is_update_field |= ImGui::SliderFloat("Lz", &gui.domain.length.z, 0.5f, 10.0f);

		ImGui::Spacing();
		is_update_marching_cube |= ImGui::SliderFloat("Isovalue", &gui.isovalue, 0.0f, 10.0f);

		ImGui::Spacing();
		is_save_obj = ImGui::Button("Export mesh as obj");
	}

	if (ImGui::CollapsingHeader("Field Function"))
	{
		// ImGui::Text("Floor");
		//is_update_field |= ImGui::SliderFloat("Floor Att Dist", &field_function.floor_att_dist, 3.0f, 30.0f);
		is_update_field |= ImGui::SliderFloat("Persistency", &field_function.floor_perlin.persistency, 0.1f, .9f);
		is_update_field |= ImGui::SliderFloat("Frequency Gain", &field_function.floor_perlin.frequency_gain, 0.1f, 10.0f);
		is_update_field |= ImGui::SliderInt("Octave", &field_function.floor_perlin.octave, 1, 5);
		is_update_field |= ImGui::SliderFloat("Scale", &field_function.floor_perlin.scale, 0.1f, 2.5f);
		is_update_field |= ImGui::SliderFloat("Mult", &field_function.floor_perlin.multiplier, 0.1f, 3.0f);
		is_update_field |= ImGui::SliderFloat("Offset", &field_function.floor_perlin.offset, 0.0f, 10.0f);
	}
}

void implicit_surface_structure::gui_update(environment_structure& gui, field_function_structure& field_function)
{
	bool is_update_marching_cube = false;
	bool is_update_field = false;
	bool is_save_obj = false;

	display_gui_implicit_surface(is_update_field, is_update_marching_cube, is_save_obj, gui, field_function);

	if (is_update_marching_cube) 
		update_marching_cube(field_function, gui.isovalue);
	if (is_update_field) {
		set_domain(gui.domain.resolution, gui.domain.length);
		update_field(field_function, gui.isovalue);
	}

	if (is_save_obj) {
		data_param.position.resize(data_param.number_of_vertex);
		data_param.normal.resize(data_param.number_of_vertex);
		save_file_obj("mesh.obj", data_param.position, data_param.normal);
	}
}

grid_3D<float> compute_discrete_scalar_field(spatial_domain_grid_3D const& domain, field_function_structure const& func)
{
	grid_3D<float> field;
	field.resize(domain.samples);

	// Fill the discrete field values
	for (int kz = 0; kz < domain.samples.z; kz++) {
		for (int ky = 0; ky < domain.samples.y; ky++) {
			for (int kx = 0; kx < domain.samples.x; kx++) {

				vec3 const p = domain.position({ kx, ky, kz });
				field(kx, ky, kz) = func(p);

			}
		}
	}

	return field;
}



grid_3D<vec3> compute_gradient(grid_3D<float> const& field)
{
	grid_3D<vec3> gradient;
	gradient.resize(field.dimension);

	int const Nx = field.dimension.x;
	int const Ny = field.dimension.y;
	int const Nz = field.dimension.z;

	// Use simple finite differences
	//  g(k) = g(k+1)-g(k) // for k<N-1
	//  otherwise g(k) = g(k)-g(k-1)

	for (int kz = 0; kz < Nz; ++kz) {
		for (int ky = 0; ky < Ny; ++ky) {
			for (int kx = 0; kx < Nx; ++kx) {

				vec3& g = gradient.at_unsafe(kx, ky, kz);
				float const f = field.at_unsafe(kx, ky, kz);

				g.x = kx != Nx - 1 ? field.at_unsafe(kx + 1, ky, kz) - f : f - field.at_unsafe(kx - 1, ky, kz);
				g.y = ky != Ny - 1 ? field.at_unsafe(kx, ky + 1, kz) - f : f - field.at_unsafe(kx, ky - 1, kz);
				g.z = kz != Nz - 1 ? field.at_unsafe(kx, ky, kz + 1) - f : f - field.at_unsafe(kx, ky, kz - 1);

			}
		}
	}

	return gradient;
}
