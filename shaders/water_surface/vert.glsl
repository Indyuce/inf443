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

// This shader uses Perlin noise to generate small bumps on the water 
// surface creating realistic specular highlights.
//
// References
// - Simplex 2D apdated from https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
// - Perlin noise taken from CGP
/***************************************************************************************************/
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

struct perlin_noise_params
{
	float persistency;
	float frequency_gain;
	int octave;
	float scale;
	float mult;
    float speed;
    float normal_boost;
};

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187, 0.366025403784439,
        -0.577350269189626, 0.024390243902439);
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod(i, 289.0);
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
    + i.x + vec3(0.0, i1.x, 1.0 ));
    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

float noise_perlin(vec2 p, perlin_noise_params params)
{
    float value = 0.0f;
    float a     = 1.0f;                // current magnitude
    float f     = 1.0f * params.scale; // current frequency

    for (int k = 0; k < params.octave; k++) {
        float n = snoise(p * f + params.speed * time);
        value += a * (0.5f + 0.5f * n);
        f *= params.frequency_gain;
        a *= params.persistency;
    }

    return value * params.mult;
}

// High octave is required for small bumps/ridges.
perlin_noise_params surface_ridges = perlin_noise_params(0.8f, 1.3f, 7, .02f, 1.0f, 0.5f, 10.0f);

// Gerstner waves are a realistic modelisation of ocean waves. These
// are superpositions of both vertical and horizontal oscillations,
// handled by vertex shaders with reduced extra performance cost.
//
// References
// - Methods adapted from https://github.com/CaffeineViking/osgw/blob/master/share/shaders/gerstner.glsl
/***************************************************************************************************/

struct gerstner_wave {
    vec2 direction;
    float amplitude;
    float steepness;
    float frequency;
    float speed;
};

// List of waves
bool waves_enabled = true;
uniform gerstner_wave gerstner_waves[3];

vec3 gerstner_wave_position(vec3 position) {
    vec3 wave_position = position.xyz;

    // Big waves
    if (waves_enabled)
        for (int i = 0; i < gerstner_waves.length(); ++i) {
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

    // Surface ridges
    float ridge_noise = noise_perlin(position.xy, surface_ridges);
    wave_position.z += ridge_noise;
    
    return wave_position;
}

/*
vec3 estimate_normal(vec2 pos) {

    // Small increments
    float incr = .0001;
    vec2 dx    = vec2(incr, 0);
    vec2 dy    = vec2(0, incr);

    // Z differences
    float dz_x = surface_ridges.normal_boost * (noise_perlin(pos + dx, surface_ridges) - noise_perlin(pos - dx, surface_ridges));
    float dz_y = surface_ridges.normal_boost * (noise_perlin(pos + dy, surface_ridges) - noise_perlin(pos - dy, surface_ridges));
    
    // Vectors
    vec3 v_x = vec3(2 * incr, 0, dz_x);
    vec3 v_y = vec3(0, 2 * incr, dz_y);

    // Normal is cross product of previous vectors
    return cross(v_x, v_y);
}
*/

mat3 rotation_matrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s, 
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s, 
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

vec3 rotate(vec3 v, vec3 axis, float angle) {
	return rotation_matrix(axis, angle) * v;
}

vec3 gerstner_wave_normal(vec3 position) {
    vec3 wave_normal = vec3(0.0, 0.0, 1.0);
    
    // Big waves
    if (waves_enabled)
        for (int i = 0; i < gerstner_waves.length(); ++i) {
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

    // Surface ridges. Only modify normals, vertices are not really required
    float incr = .001;
    vec2 pos   = position.xy;
    vec2 dx    = vec2(incr, 0);
    vec2 dy    = vec2(0, incr);
    float dz_x = noise_perlin(pos + dx, surface_ridges) - noise_perlin(pos - dx, surface_ridges);
    float dz_y = noise_perlin(pos + dy, surface_ridges) - noise_perlin(pos - dy, surface_ridges);
    float angle_x = atan(dz_x, 2 * incr);
    float angle_y = atan(dz_y, 2 * incr);
    
    // TODO optimizable
    wave_normal = rotate(wave_normal, vec3(0, -1, 0), angle_x);
    wave_normal = rotate(wave_normal, vec3(1, 0, 0), angle_y);
    
    return normalize(wave_normal);
}

// Water surface vertex shader
/***************************************************************************************************/

void main()
{
	vec4 position = model * vec4(vertex_position, 1.0); // Position in world space
    vec4 normal = modelNormal * vec4(vertex_normal, 0.0);

	// Gerstner waves + Perlin noise
    vec3 wave_position = gerstner_wave_position(position.xyz);
    vec3 wave_normal = gerstner_wave_normal(position.xyz);

	// Fill the parameters sent to the fragment shader
	fragment.position = wave_position;
	fragment.normal   = wave_normal;
	fragment.color    = vertex_color;
	fragment.uv       = vertex_uv;

	// gl_Position is a built-in variable which is the expected output of the vertex shader
	// The projected position of the vertex in the normalized device coordinates
	gl_Position = projection * view * vec4(wave_position.xyz, position.w); // gl_Position is the projected vertex position (in normalized device coordinates)
}
