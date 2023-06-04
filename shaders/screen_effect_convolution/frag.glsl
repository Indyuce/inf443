#version 330 core

in vec2 uv_frag;

uniform sampler2D image_texture;
uniform sampler2D extra_texture;
uniform sampler2D bright_texture;

layout(location=0) out vec4 FragColor;

vec3 simple_blur(int blur_radius) {
	int samples = 2 * blur_radius + 1;
	float weight = 1.0f / float(samples * samples);

	ivec2 image_size = textureSize(image_texture, 0);
	float dx = 1.0 / float(image_size.x);
	float dy = 1.0 / float(image_size.y);

	vec3 sum = vec3(0, 0, 0);

	for (int i = -blur_radius; i <= blur_radius; i++)
		for (int j = -blur_radius; j <= blur_radius; j++)
			sum += texture(image_texture, uv_frag + vec2(float(i) * dx, float(j) * dx)).xyz;

	return sum * weight;

}

void main()
{
	float depth = texture(extra_texture, uv_frag).x;
	vec3 current_color = texture(image_texture, uv_frag).xyz;
	 
	// Simple Blur TODO gaussian with multipass.
	//**************************************************************************************//
	int blur_radius = int(floor(10.0f * depth));
	if (blur_radius > 0)
		current_color = simple_blur(blur_radius);

	// Bloom (looks horrible)
	//**************************************************************************************//
	//vec3 bloom_color = simple_blur(true, 10);
	//const float gamma = 2.2f;
	//const float exposure = 1.1f;
    //current_color += bloom_color; // additive blending
    //vec3 result = vec3(1.0f) - exp(-current_color * exposure); // tone mapping
    //current_color = pow(result, vec3(1.0f / gamma)); // also gamma correct while we're at it

	FragColor = vec4(current_color, 1.0f);
}
