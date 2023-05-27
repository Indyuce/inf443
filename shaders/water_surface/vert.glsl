#version 330 core

// Vertex shader - this code is executed for every vertex of the shape

// Inputs coming from VBOs
layout (location = 0) in vec3 vertex_position; // vertex position in local space (x,y,z)
layout (location = 1) in vec3 vertex_normal;   // vertex normal in local space   (nx,ny,nz)
layout (location = 2) in vec3 vertex_color;    // vertex color      (r,g,b)
layout (location = 3) in vec2 vertex_uv;       // vertex uv-texture (u,v)

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
uniform float time;

struct gerstner_wave {
    vec2 direction;
    float amplitude;
    float steepness;
    float frequency;
    float speed;
};

// List of waves
int gerstner_waves_length = 10; // TODO use array.length()
gerstner_wave gerstner_waves[10];

vec3 gerstner_wave_position(vec3 position, float time) {
    vec3 wave_position = position.xyz;
    
    for (int i = 0; i < gerstner_waves_length; ++i) {
        float proj = dot(position.xy, gerstner_waves[i].direction),
              phase = time * gerstner_waves[i].speed,
              theta = proj * gerstner_waves[i].frequency + phase,
              height = gerstner_waves[i].amplitude * sin(theta);

        wave_position.z += height;

        float maximum_width = gerstner_waves[i].steepness *
                              gerstner_waves[i].amplitude,
              width = maximum_width * cos(theta),
              x = gerstner_waves[i].direction.x,
              y = gerstner_waves[i].direction.y;

        wave_position.x += x * width;
        wave_position.y += y * width;
    }
    
    return wave_position;
}

vec3 gerstner_wave_normal(vec3 position, float time) {
    vec3 wave_normal = vec3(0.0, 0.0, 1.0);

    for (int i = 0; i < gerstner_waves_length; ++i) {
        float proj = dot(position.xz, gerstner_waves[i].direction),
              phase = time * gerstner_waves[i].speed,
              psi = proj * gerstner_waves[i].frequency + phase,
              Af = gerstner_waves[i].amplitude *
                   gerstner_waves[i].frequency,
              alpha = Af * sin(psi);

        wave_normal.z -= gerstner_waves[i].steepness * alpha;

        float x = gerstner_waves[i].direction.x,
              y = gerstner_waves[i].direction.y,
              omega = Af * cos(psi);

        wave_normal.x -= x * omega;
        wave_normal.y -= y * omega;
    }
    
    return wave_normal;
}

void main()
{
    // Wave parameters
    float angle_init = 0.0f;
    float angle_diff = 16.45f * 3.14159265f / 180.0f;
    float amplitude_init        = 0.1f;
    float amplitude_persistance = .6f;
    float steepness_init        = 0.5f;
    float steepness_persistance = 0.9f;
    float frequency_init        = 0.1f;
    float frequency_gain        = 1.3f;
    float speed_init            = 0.5f;
    float speed_gain            = 1.3f;

    // Proceduraly generated wave parameters like Perlin noise
    for (int i = 0; i < gerstner_waves_length; ++i) {
        vec2 dir = vec2(cos(angle_init), sin(angle_init));
        gerstner_waves[i] = gerstner_wave(dir, amplitude_init, steepness_init, frequency_init, speed_init);
        
        angle_init     += angle_diff;
        amplitude_init *= amplitude_persistance;
        steepness_init *= steepness_persistance;
        frequency_init *= frequency_gain;
        speed_init     *= speed_gain;
    }

	vec4 position = model * vec4(vertex_position, 1.0); // Position in world space
    vec4 normal = modelNormal * vec4(vertex_normal, 0.0);

	// Gerstner waves
    vec3 wave_position = gerstner_wave_position(position.xyz, time);
    vec3 wave_normal = gerstner_wave_normal(position.xyz, time);

	// Fill the parameters sent to the fragment shader
	fragment.position = wave_position;
	fragment.normal   = wave_normal;
	fragment.color    = vertex_color;
	fragment.uv       = vertex_uv;

	// gl_Position is a built-in variable which is the expected output of the vertex shader
	// The projected position of the vertex in the normalized device coordinates
	gl_Position = projection * view * vec4(wave_position.xyz, position.w); // gl_Position is the projected vertex position (in normalized device coordinates)
}
