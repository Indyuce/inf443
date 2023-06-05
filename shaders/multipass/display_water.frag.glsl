#version 330 core

in vec2 uv_frag;

uniform sampler2D image_texture;
uniform sampler2D extra_texture;
uniform sampler2D bright_texture;
uniform float bloom_threshold;

layout(location=0) out vec4 FragColor;
layout(location=1) out vec4 ExtraColor;
layout(location=2) out vec4 BrightColor;

void main()
{
	FragColor = texture(image_texture, uv_frag);
	ExtraColor = texture(extra_texture, uv_frag);

	// Check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > bloom_threshold)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
