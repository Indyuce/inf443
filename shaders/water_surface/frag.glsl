#version 330 core

// Inputs coming from the vertex shader
in struct fragment_data
{
    vec3 position; // position in the world space
    vec3 normal;   // normal in the world space
    vec3 color;    // current color on the fragment
    vec2 uv;       // current uv-texture on the fragment

} fragment;

// Output of the fragment shader - output color
layout(location=0) out vec4 FragColor;
layout(location=1) out vec4 ExtraColor;
layout(location=2) out vec4 BrightColor;

// View matrix
uniform mat4 view;
uniform vec3 camera_position;
uniform vec3 camera_direction;

struct material_structure
{
	vec3 color;  // Uniform color of the object
};
uniform material_structure material;

// Ambiant uniform controled from the GUI
uniform float ambiant;
uniform float diffuse;
uniform float specular;
uniform int specular_exp;

uniform float flashlight;
uniform int flashlight_exp;
uniform float flashlight_dist;
uniform vec3 flashlight_color;

uniform vec3 light_color;
uniform vec3 light_direction;

uniform vec3 fog_color;
uniform bool surf_height;
uniform float floor_level;
uniform float scale;
uniform float water_attenuation_coefficient;
uniform float water_optical_index;
uniform float time;

struct gerstner_wave {
    vec2 direction;
    float amplitude;
    float steepness;
    float frequency;
    float speed;
};

uniform samplerCube image_skybox;
uniform sampler2D texture_sand;
uniform float sand_texture_scale;
uniform float fog_distance;
uniform float render_distance;

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

// Depth buffer calculation
/***************************************************************************************************/
uniform float depth_min;
uniform float depth_max;

float get_depth_buffer(float frag_distance) {
	return (frag_distance - depth_min) / depth_max;
}

// Main water surface Shader
/***************************************************************************************************/

vec3 water_attenuation(vec3 current_color, float attenuation_distance) {

  return mix(fog_color, current_color, exp(-water_attenuation_coefficient * scale * attenuation_distance));
}

vec3 get_wave_normal(vec3 position) {
    float incr = .001;
    vec2 pos   = position.xy;

    vec2 dx    = vec2(incr, 0);
    vec2 dy    = vec2(0, incr);

    float dz_x = noise_perlin(pos + dx, surface_ridges) - noise_perlin(pos - dx, surface_ridges);
    float dz_y = noise_perlin(pos + dy, surface_ridges) - noise_perlin(pos - dy, surface_ridges);

    float denom = 2 * incr;
    return normalize(vec3(dz_x / denom, dz_y / denom, 1));
}

// ----------------------------------------------------------------------------------
// OPTIMISATION: avoid use of length() and normalize() or cache the results
// Not much changed.
// ----------------------------------------------------------------------------------
void main()
{
    // Height map shader for debug
    /***********************************************************/
    if (surf_height) {
        float p = fragment.position.z;
        vec3 color1 = vec3(0.012,0.478,0.871);
        vec3 color2 = vec3(0.012,0.898,0.718);
        FragColor = vec4(p * color2 + (1 - p) * color1, 1.0);
        return;
    }

    vec3 current_color = vec3(0, 0, 0);
    float opacity = 1.0;

    // Prepare for refraction/reflection
    float distance_to_water = length(fragment.position - camera_position);
    vec3 I = (fragment.position - camera_position) / distance_to_water;

    // Underwater
    /***********************************************************/
    if (gl_FrontFacing == false) {
    
        // ----------------------------------------------------------------------------------
        // OPTIMISATION: over 5 caracteristic attenuation distances, just display fog
        // Reduces by up to ~60% GPU utilization underwater when looking at the surface.
        // ----------------------------------------------------------------------------------
        float total_attenuation_distance = 5.0f / (scale * water_attenuation_coefficient);
        if (distance_to_water > total_attenuation_distance) {
            FragColor = vec4(fog_color, 1.0f);
            return;
        }

		vec3 N = -get_wave_normal(fragment.position); // NORMAl CALCULATION.
        vec3 texture_coords = refract(I, N, water_optical_index);

        // Total reflection
        if (texture_coords.x == 0 && texture_coords.y == 0 && texture_coords.z == 0) {
            vec3 reflected_dir = reflect(I, N);
            current_color = texture(texture_sand, reflected_dir.xy).xyz;
            float correction_coefficient = .2f;
            current_color = water_attenuation(current_color, abs(floor_level / reflected_dir.z) * correction_coefficient);
        }

        // Inside of Snell's window!!!
        else
            current_color = texture(image_skybox, texture_coords).xyz;

        // Absorption
        current_color = water_attenuation(current_color, distance_to_water);
	}
    
    // Above Surface Level
    /***********************************************************/
    else {
        
        // TODO atmosphere shader?
        float fog_coefficient = min(distance_to_water / fog_distance, 1.0f);
        vec3 far_away_for_color = vec3(1.0f, 1.0f, 1.0f);

        // ----------------------------------------------------------------------------------
        // OPTIMISATION: over 95% fog attenuation, just display far-away air fog.
        // Adding far-away fog reduced GPU utilization by ~5%, not much
        // However, a very small fog distance can easily halve GPU utilization.
        // ----------------------------------------------------------------------------------
        if (fog_coefficient > .95) {
            current_color = far_away_for_color;

        } else {
            // Partial reflection - Water reflects at normal angles and refracts more at steep angles
            // Opacity also corresponds to the angle steepness
		    vec3 N = get_wave_normal(fragment.position); // NORMAl CALCULATION.
            opacity = 1 - min(0.2f, pow(max(0, dot(N, -I)), 3)); // Fresnel effect. Source: https://www.youtube.com/watch?v=vTMEdHcKgM4&t=874s
            opacity = max(opacity, fog_coefficient); // Fix fog.
            current_color = texture(image_skybox, reflect(I, N)).xyz; // Water texture is reflection. TODO better handling of refraction with multipass.
            current_color = mix(current_color, far_away_for_color, fog_coefficient);
        }
    }

    // Color attenuation

    // Specular sunlight
    // TODO
    // float specular_magnitude = pow(max(0, dot(R, Cn)), specularExp) * specular;
    // current_color += specular_magnitude * light_color;
    
	// Texture outputs
    /************************************************************/
    FragColor = vec4(current_color, opacity); // Note: the last alpha component is not used here
	ExtraColor = vec4(get_depth_buffer(distance_to_water), 0.0, 0.0, 0.0); // Output extra buffers

    // check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
