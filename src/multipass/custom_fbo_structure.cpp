#include "custom_fbo_structure.hpp"

using namespace cgp;

void custom_fbo_structure::initialize() {

	width = 640;
	height = 480;

	// Initialize texture
	texture.initialize_texture_2d_on_gpu(width, height, GL_RGB8, GL_TEXTURE_2D);
	texture_extra.initialize_texture_2d_on_gpu(width, height, GL_RGB8, GL_TEXTURE_2D); // new
	texture_bright.initialize_texture_2d_on_gpu(width, height, GL_RGB8, GL_TEXTURE_2D); // new

	// Allocate a depth buffer - need to do it when using the frame buffer
	glGenRenderbuffers(1, &depth_buffer_id);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_id);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, max_width, max_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Create frame buffer
	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	// associate textures to the frame buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0); // new
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture_extra.id, 0); // new
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, texture_bright.id, 0); // new
	// tell to write to multiple textures (NEW)
	GLenum DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, DrawBuffers);
	// associate the depth-buffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_id);
	// Reset the standard framebuffer to output on the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void custom_fbo_structure::bind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	//texture.bind(); Useless // new
}

void custom_fbo_structure::unbind() const {
	//texture.unbind(); Useless // new
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void custom_fbo_structure::update_screen_size(int new_width, int new_height) {

	if (width != new_width || height != new_height)
	{
		width = new_width;
		height = new_height;

		glBindTexture(GL_TEXTURE_2D, texture.id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glBindTexture(GL_TEXTURE_2D, texture_extra.id); // new
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // new
		glBindTexture(GL_TEXTURE_2D, texture_bright.id); // new
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL); // new
		glBindTexture(GL_TEXTURE_2D, 0);
	}

}