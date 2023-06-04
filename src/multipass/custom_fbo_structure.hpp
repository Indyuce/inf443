#pragma once

#include "cgp/cgp.hpp"


// Just the CGP opengl_fbo_structure but with two textures associated to it!!!
struct custom_fbo_structure {

	// ID of the FBO
	GLuint id;

	// Depth buffer
	GLuint depth_buffer_id;

	// Texture storing the rendering in this FBO
	// References for MRT:
	// - https://github.com/xvde110/opengl-Bloom/blob/master/Bloom/Bloom.cpp
	// - https://learnopengl.com/Advanced-OpenGL/Framebuffers
	// - https://learnopengl.com/Advanced-Lighting/Bloom
	cgp::opengl_texture_image_structure texture, texture_extra, texture_bright;

	// Size of the texture
	int width;
	int height;

	// Initialize the ids and the texture
	//  This function must be called before any rendering pass
	void initialize();

	// Start the rendering pass where the output will be stored on the FBO
	void bind() const;
	// Stop the rendering pass on the FBO
	void unbind() const;

	// Update the screen size (resize the texture if needed)
	void update_screen_size(int window_width, int windows_height);

	// Max size for the depth-map
	static const int max_width = 3840;
	static const int max_height = 2160;
};