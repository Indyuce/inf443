#include "multipass_structure.hpp"

using namespace cgp;

void multipass_structure::initialize(std::string project_path)
{
	// Initialize the FBO for the first pass
	fbo_pass_1.initialize();
	fbo_pass_2.initialize();

	// Initialize a simple quad used to display the result on the entire screen for 2nd pass
	quad_pass_2.initialize_data_on_gpu(mesh_primitive_quadrangle({ -1,-1,0 }, { 1,-1,0 }, { 1,1,0 }, { -1,1,0 }));
	quad_pass_3 = quad_pass_2;

	// Set input texture of pass 2 <= Output texture output of pass1
	quad_pass_2.texture = fbo_pass_1.texture;
	quad_pass_2.supplementary_texture["extra_texture"] = fbo_pass_1.texture_extra;
	quad_pass_2.supplementary_texture["bright_texture"] = fbo_pass_1.texture_bright;
	quad_pass_3.texture = fbo_pass_2.texture;
	quad_pass_3.supplementary_texture["extra_texture"] = fbo_pass_1.texture_extra;
	quad_pass_3.supplementary_texture["bright_texture"] = fbo_pass_1.texture_bright;

	quad_pass_2.shader.load(
		project_path + "shaders/multipass/display_water.vert.glsl",
		project_path + "shaders/multipass/display_water.frag.glsl"
	);

	quad_pass_3.shader.load(
		project_path + "shaders/multipass/post_process.vert.glsl",
		project_path + "shaders/multipass/post_process.frag.glsl"
	);

	// Have second pass (water surface) also write on extra buffers
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_pass_2.id);
	// Associate same textures to second buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fbo_pass_1.texture_extra.id, 0); // new
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fbo_pass_1.texture_bright.id, 0); // new
	// tell to write to multiple textures (NEW)
	GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, DrawBuffers);
	// Associate same depth buffer as first
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo_pass_1.depth_buffer_id);
	// Reset the standard framebuffer to output on the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void multipass_structure::update_screen_size(int width, int height)
{
	fbo_pass_1.update_screen_size(width, height);
	fbo_pass_2.update_screen_size(width, height);
}

void multipass_structure::clear_screen()
{
	glViewport(0, 0, fbo_pass_1.width, fbo_pass_1.height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
}

void multipass_structure::start_pass_1()
{
	fbo_pass_1.bind();
	clear_screen();
}

void multipass_structure::end_pass_1()
{
	fbo_pass_1.unbind();
}

void multipass_structure::start_pass_2() {
	fbo_pass_2.bind();
}

void multipass_structure::draw_pass_2(environment_generic_structure const& environment)
{
	glDisable(GL_DEPTH_TEST);
	draw(quad_pass_2, environment, false);
	glEnable(GL_DEPTH_TEST);
}

void multipass_structure::end_pass_2()
{
	fbo_pass_2.unbind();
}

void multipass_structure::start_pass_3() {
	clear_screen();
}

void multipass_structure::draw_pass_3(environment_generic_structure const& environment)
{
	glDisable(GL_DEPTH_TEST);
	draw(quad_pass_3, environment, false);
	glEnable(GL_DEPTH_TEST);
}

void multipass_structure::end_pass_3()
{

}