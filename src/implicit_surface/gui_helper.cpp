#include "gui_helper.hpp"

using namespace cgp;

void display_gui_implicit_surface(bool& is_update_field, bool& is_update_marching_cube, bool& is_save_obj, gui_parameters& gui, field_function_structure& field_function)
{
	if (ImGui::CollapsingHeader("Marching Cube"))
	{
		is_update_field |= ImGui::SliderInt("Samples", &gui.domain.samples, 8, 100);

		//is_update_field |= ImGui::SliderFloat("Lx", &gui.domain.length.x, 0.5f, 10.0f);
		//is_update_field |= ImGui::SliderFloat("Ly", &gui.domain.length.y, 0.5f, 10.0f);
		//is_update_field |= ImGui::SliderFloat("Lz", &gui.domain.length.z, 0.5f, 10.0f);

		ImGui::Spacing();
		is_update_marching_cube |= ImGui::SliderFloat("Isovalue", &gui.isovalue, 0.0f, 1.0f);

		ImGui::Spacing();
		is_save_obj = ImGui::Button("Export mesh as obj");
	}

	if (ImGui::CollapsingHeader("Field Function"))
	{
		// ImGui::Text("Floor");
		//is_update_field |= ImGui::SliderFloat("Floor Att Dist", &field_function.floor_att_dist, 3.0f, 30.0f);
		is_update_field |= ImGui::SliderFloat("Persistency", &field_function.cave_perlin.persistency, 0.1f, .9f);
		is_update_field |= ImGui::SliderFloat("Frequency Gain", &field_function.cave_perlin.frequency_gain, 0.1f, 10.0f);
		is_update_field |= ImGui::SliderInt("Octave", &field_function.cave_perlin.octave, 1, 5);
		is_update_field |= ImGui::SliderFloat("Scale", &field_function.cave_perlin.scale, 0.1f, 2.5f);
		is_update_field |= ImGui::SliderFloat("Mult", &field_function.cave_perlin.multiplier, 5.0f, 12.0f);
		is_update_field |= ImGui::SliderFloat("Offset", &field_function.cave_perlin.offset, 0.0f, 3.0f);
	}
}