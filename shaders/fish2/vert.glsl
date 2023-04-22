#version 330 core

// Vertex shader - this code is executed for every vertex of the shape

// Inputs coming from VBOs
layout(location = 0) in vec3 vertex_position; // vertex position in local space (x,y,z)
layout(location = 1) in vec3 vertex_normal;   // vertex normal in local space   (nx,ny,nz)
layout(location = 2) in vec3 vertex_color;    // vertex color      (r,g,b)
layout(location = 3) in vec2 vertex_uv;       // vertex uv-texture (u,v)
uniform float time;
uniform vec3 head_position;
uniform vec3 direction;
uniform float frequency;
// Output variables sent to the fragment shader
out struct fragment_data
{
	vec3 position; // vertex position in world space
	vec3 normal;   // normal position in world space
	vec3 color;    // vertex color
	vec2 uv;       // vertex uv
} fragment;

// Uniform variables expected to receive from the C++ program
uniform mat4 model; // Model affine transform matrix associated to the current shape
uniform mat4 view;  // View matrix (rigid transform) of the camera
uniform mat4 projection; // Projection (perspective or orthogonal) matrix of the camera

uniform mat4 modelNormal; // Model without scaling used for the normal. modelNormal = transpose(inverse(model))

mat3 rotationMatrix(vec3 axis, float angle)
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;

	return mat3(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s,
		oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s,
		oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c);
}
float rand(float val)
{
	return sin(val * 12.9898 * 43758.5453);
}

void main()
{



	float amplitude_coef = 0.1;
	float vertical_amplitude = 0.15;

	float amplitude = 0;
	vec3 real_vertex_position = vec3(vertex_position[2], -vertex_position[1], -vertex_position[0]);
	if (real_vertex_position[2] < -0.2)
		amplitude = amplitude_coef * pow(real_vertex_position[2], 2);
	real_vertex_position += amplitude * sin(frequency * (100 + time)) * vec3(1, 0, 0);
	real_vertex_position += +vertical_amplitude * sin(0.3 * frequency * (100 + time) + real_vertex_position[2]) * vec3(0, 1, 1);

	// The position of the vertex in the world space
	vec4 position = model * vec4(real_vertex_position, 1.0);

	// The normal of the vertex in the world space
	vec4 normal = modelNormal * vec4(vertex_normal, 0.0);

	// The projected position of the vertex in the normalized device coordinates:
	vec4 position_projected = projection * view * position;

	// Fill the parameters sent to the fragment shader
	fragment.position = real_vertex_position.xyz;
	fragment.normal = normal.xyz;
	fragment.color = vertex_color;
	fragment.uv = vertex_uv;

	//fragment.uv[0] = 1.-fragment.uv[0];

	// gl_Position is a built-in variable which is the expected output of the vertex shader
	gl_Position = position_projected; // gl_Position is the projected vertex position (in normalized device coordinates)
}
