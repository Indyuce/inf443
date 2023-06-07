#version 330 core

#define PI 3.141592
#define iSteps 16
#define jSteps 8

// Inputs coming from the vertex shader
in struct fragment_data
{
    vec3 position; // position in the world space
    vec3 normal;   // normal in the world space
    vec3 color;    // current color on the fragment
    vec2 uv;       // current uv-texture on the fragment

} fragment;

in vec4 clip_space;

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
uniform float direct;
uniform int direct_exp;

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

uniform samplerCube texture_skybox;
uniform sampler2D texture_scene;
uniform sampler2D texture_sand;
uniform sampler2D texture_extra;
uniform float sand_texture_scale;
uniform float fog_distance;
uniform float render_distance;
uniform float bloom_threshold;

// Atmosphere shader
// References:
// - https://github.com/wwwtyro/glsl-atmosphere (adapted from here)
// - https://www.youtube.com/watch?v=DxfEbulyFcY&t=794s (calculations)
/***************************************************************************************************/

uniform float psdMie;
uniform float shMie;
uniform float shRlh;
uniform float kMie;
uniform vec3 kRlh;
uniform float iSun;
uniform bool atmos_shader;

vec2 rsi(vec3 r0, vec3 rd, float sr) {
    // ray-sphere intersection that assumes
    // the sphere is centered at the origin.
    // No intersection when result.x > result.y
    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, r0);
    float c = dot(r0, r0) - (sr * sr);
    float d = (b*b) - 4.0*a*c;
    if (d < 0.0) return vec2(1e5,-1e5);
    return vec2(
        (-b - sqrt(d))/(2.0*a),
        (-b + sqrt(d))/(2.0*a)
    );
}

vec3 atmosphere(vec3 fragment_direction, bool sun_visible) {

    // Initialize variables
    vec3 r = fragment_direction.xzy;
    vec3 pSun = -light_direction.xzy;
    vec3 r0 = vec3(0, 0, 6371e3).xzy;
    float rPlanet = 6371e3;
    float rAtmos = 6471e3;

    // Calculate the step size of the primary ray.
    vec2 p = rsi(r0, r, rAtmos);
    if (p.x > p.y) return vec3(1, 1, 1);
    p.y = min(p.y, rsi(r0, r, rPlanet).x);
    float iStepSize = (p.y - p.x) / float(iSteps);

    // Initialize the primary ray time.
    float iTime = 0.0;

    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 totalRlh = vec3(0,0,0);
    vec3 totalMie = vec3(0,0,0);

    // Initialize optical depth accumulators for the primary ray.
    float iOdRlh = 0.0;
    float iOdMie = 0.0;

    // Calculate the Rayleigh and Mie phases.
    float mu = dot(r, pSun);
    float mumu = mu * mu;
    float g = psdMie;
    float gg = g * g;
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));

    // Sample the primary ray.
    for (int i = 0; i < iSteps; i++) {

        // Calculate the primary ray sample position.
        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);

        // Calculate the height of the sample.
        float iHeight = length(iPos) - rPlanet;

        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
        float odStepMie = exp(-iHeight / shMie) * iStepSize;

        // Accumulate optical depth.
        iOdRlh += odStepRlh;
        iOdMie += odStepMie;

        // Calculate the step size of the secondary ray.
        float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);

        // Initialize the secondary ray time.
        float jTime = 0.0;

        // Initialize optical depth accumulators for the secondary ray.
        float jOdRlh = 0.0;
        float jOdMie = 0.0;

        // Sample the secondary ray.
        for (int j = 0; j < jSteps; j++) {

            // Calculate the secondary ray sample position.
            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);

            // Calculate the height of the sample.
            float jHeight = length(jPos) - rPlanet;

            // Accumulate the optical depth.
            jOdRlh += exp(-jHeight / shRlh) * jStepSize;
            jOdMie += exp(-jHeight / shMie) * jStepSize;

            // Increment the secondary ray time.
            jTime += jStepSize;
        }

        // Calculate attenuation.
        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

        // Accumulate scattering.
        totalRlh += odStepRlh * attn;
        totalMie += odStepMie * attn;

        // Increment the primary ray time.
        iTime += iStepSize;

    }

    // Atmosphere color
    vec3 atmos_color = iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);
    atmos_color = 1.0 - exp(-1.0 * atmos_color); // Fix exposition

    // Direct sunlight
    if (sun_visible) {
        float direct_magnitude = direct * pow(max(dot(fragment_direction, -light_direction), 0), direct_exp);
        atmos_color += direct_magnitude * light_color;
    }

    // Calculate and return the final color.
    return atmos_color;
}

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
perlin_noise_params surface_ridges = perlin_noise_params(0.8f, 1.3f, 7, .02f, 1.6f, 0.5f);

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

void set_outputs(vec3 current_color, float depth_buffer) {
    FragColor = vec4(current_color, 1.0f);
    ExtraColor = vec4(depth_buffer, 0.0, 0.0, 0.0); // Output extra buffers

    // Check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > bloom_threshold)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

// Better handling of fog with atmospheric shader
vec3 get_fog_color(vec3 direction) {

    // Default fog color
    if (!atmos_shader) return vec3(1.0f, 1.0f, 1.0f);

    // Better fog.
    vec3 fixed_direction = normalize(vec3(direction.xy, 0));
    return atmosphere(fixed_direction, false);
}

// ----------------------------------------------------------------------------------
// OPTIMISATION: avoid use of length() and normalize() or cache the results
// Not much changed.
// ----------------------------------------------------------------------------------
void main()
{
    float distance_to_water = length(fragment.position - camera_position);
    float depth_buffer = get_depth_buffer(distance_to_water);

    // Height map shader for debug
    /***********************************************************/
    if (surf_height) {
        float p = fragment.position.z;
        vec3 color1 = vec3(0.012,0.478,0.871);
        vec3 color2 = vec3(0.012,0.898,0.718);
        set_outputs(p * color2 + (1 - p) * color1, depth_buffer);
        return;
    }
    
    // Main Shader
    /***********************************************************/

    // Prepare for refraction/reflection
    vec3 current_color = vec3(0, 0, 0);
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
            set_outputs(fog_color, depth_buffer);
            return;
        }

		vec3 N = -get_wave_normal(fragment.position); // NORMAL CALCULATION
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
            current_color = atmos_shader ? atmosphere(texture_coords, true) : texture(texture_skybox, texture_coords).xyz;

        // Absorption
        current_color = water_attenuation(current_color, distance_to_water);
	}
    
    // Above Surface Level
    /***********************************************************/
    else {
        
        float fog_coefficient = min(distance_to_water / fog_distance, 1.0f);
        vec3 render_fog = get_fog_color(I);

        // ----------------------------------------------------------------------------------
        // OPTIMISATION: over 95% fog attenuation, just display far-away air fog.
        // Adding far-away fog reduced GPU utilization by ~5%, not much
        // However, a very small fog distance can easily halve GPU utilization.
        // ----------------------------------------------------------------------------------
        if (fog_coefficient > .95) {
            current_color = render_fog;

        } else {
        
		    vec3 N = get_wave_normal(fragment.position); // NORMAL CALCULATION

            // Fake diffraction. This simply offsets the refraction texture by value
            // given by the previous normal calculation.
            // Source: https://www.youtube.com/watch?v=6B7IF6GOu7s&t=343s
            vec2 refract_text_offset = N.xy * .05f;

            // This is the texture from the previous pass.
            // This is also the texture being used for refraction.
            // Source: https://www.youtube.com/watch?v=GADTasvDOX4&t=386s (projective texture mapping)
            vec3 refraction_texture = texture(texture_scene, (clip_space.xy / clip_space.w) / 2.0f + .5f + refract_text_offset).xyz;
            // Reflect skybox onto water to find the reflection texture.
            vec3 reflexion_texture = atmos_shader ? atmosphere(reflect(I, N), true) : texture(texture_skybox, reflect(I, N)).xyz;

            // Partial reflection - Water reflects at normal angles and refracts more at steep angles
            // Reflectiveness also corresponds to the angle steepness
            // Fresnel effect. Source: https://www.youtube.com/watch?v=vTMEdHcKgM4&t=874s
            float transparency = min(0.2f, pow(max(0, dot(N, -I)), 3));

            current_color = mix(reflexion_texture, refraction_texture, transparency); // Blend reflection and refraction
            current_color = mix(current_color, render_fog, fog_coefficient); // Apply fog
        }
    }

    // Color attenuation

    // Specular sunlight
    // TODO
    // float specular_magnitude = pow(max(0, dot(R, Cn)), specularExp) * specular;
    // current_color += specular_magnitude * light_color;
    
	// Texture outputs
    /************************************************************/
    set_outputs(current_color, depth_buffer);
}
