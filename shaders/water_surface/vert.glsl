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

out vec4 clip_space;

// Uniform variables expected to receive from the C++ program
uniform mat4 model; // Model affine transform matrix associated to the current shape
uniform mat4 modelNormal; // Model without scaling used for the normal. modelNormal = transpose(inverse(model))
uniform mat4 view;  // View matrix (rigid transform) of the camera
uniform mat4 projection; // Projection (perspective or orthogonal) matrix of the camera
uniform float time;
uniform float water_surface_center_length;
uniform vec3 camera_position;

// This shader uses Perlin noise to generate small bumps on
// the water surface creating realistic specular highlights.
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
perlin_noise_params surface_ridges = perlin_noise_params(0.8f, 1.3f, 7, .02f, 1.3f, 0.5f);

// Water surface vertex shader
/***************************************************************************************************/

vec3 get_wave_position(vec3 position, float height_coefficient) {
    vec3 wave_position = position.xyz;
    wave_position.z += noise_perlin(position.xy, surface_ridges) * height_coefficient;
    return wave_position;
}

void main()
{
	vec4 position = model * vec4(vertex_position, 1.0); // Position in world space
	vec4 normal = modelNormal * vec4(vertex_normal, 0.0); // Normal in world space
    
    vec3 wave_position = position.xyz;
    vec3 wave_normal   = normal.xyz;
    
    // Gradually reduce size of waves when reaching end of highly precise surface
    float distance_on_plane = length(camera_position.xy - position.xy);
    float height_coefficient = 1 - pow(2 * distance_on_plane / water_surface_center_length, 5);
    // If sufficiently far away, no need to calculate vertex height
    // Perlin noise for waves, with threshold to reduce performance cost.
    if (height_coefficient > .05)
        wave_position = get_wave_position(wave_position, height_coefficient);

	// Fill the parameters sent to the fragment shader
	fragment.position = wave_position;
	fragment.normal   = wave_normal;  // Not used
	fragment.color    = vertex_color; // Not used
	fragment.uv       = vertex_uv;    // Not used

	// gl_Position is a built-in variable which is the expected output of the vertex shader
	// The projected position of the vertex in the normalized device coordinates
    clip_space = projection * view * vec4(wave_position.xyz, position.w);
	gl_Position = clip_space; // gl_Position is the projected vertex position (in normalized device coordinates)
}
