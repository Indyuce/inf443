#version 330 core

in vec2 uv_frag;

uniform sampler2D image_texture;

layout(location=0) out vec4 FragColor;

void main()
{
	FragColor = texture(image_texture, uv_frag);
}
