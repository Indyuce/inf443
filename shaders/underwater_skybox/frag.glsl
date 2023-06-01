#version 330 core

in struct fragment_data
{
	vec3 position;
} fragment;

layout(location=0) out vec4 FragColor;

uniform samplerCube image_skybox;

uniform float direct;
uniform int direct_exp;
uniform float water_surface_plane_length;

uniform vec3 fog_color;
uniform vec3 light_color;
uniform vec3 light_direction;
uniform vec3 camera_position;
uniform vec3 camera_direction;

void main()
{
    // Texture color
    vec3 current_color = vec3(texture(image_skybox, fragment.position));

    

	FragColor = vec4(current_color, 1.0);
}