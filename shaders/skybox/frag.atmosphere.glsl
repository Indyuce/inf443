#version 330 core

#define PI 3.141592
#define iSteps 16
#define jSteps 8

in struct fragment_data
{
	vec3 position;
} fragment;

layout(location=0) out vec4 FragColor;
layout(location=1) out vec4 ExtraColor;
layout(location=2) out vec4 BrightColor;

uniform samplerCube image_skybox;

uniform float direct;
uniform int direct_exp;
uniform float water_surface_plane_length;
uniform float fog_distance;

uniform vec3 fog_color;
uniform vec3 light_color;
uniform vec3 light_direction;
uniform vec3 camera_position;
uniform vec3 camera_direction;

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

vec3 atmosphere(vec3 fragment_direction) {

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
    float direct_magnitude = direct * pow(max(dot(fragment_direction, -light_direction), 0), direct_exp);
    atmos_color += direct_magnitude * light_color;

    // Calculate and return the final color.
    return atmos_color;
}

// Main Shader
/***************************************************************************************************/

void main()
{
    // Texture color
    vec3 current_color = vec3(0.0, 0.0, 0.0);

    // Is this pixel underwater? Since water is transparent, frame
    // can have both underwater and sky pixels at the same time.
    //
    // Two conditions:
    // - Position condition: if player is inside of water, skybox is entirely blue water.
    // - Geometric condition: if z is above a certain threshold value,
    //   that depends on view distance and camera altitude, then player
    //   is looking at sky pixel.
    float water_level = 5.0f;
    vec3 fragment_direction = normalize(fragment.position);
    bool under_water_pixel = camera_position.z < water_level || fragment_direction.z < -camera_position.z / fog_distance;

    // Underwater
    /***********************************************************/
    if (under_water_pixel) {
        current_color = fog_color;
    }
    
    // Over Water Surface
    /***********************************************************/
    else {
        current_color = fragment.position.z < 0 ? vec3(1, 1, 1) : atmosphere(fragment_direction);
    }
    
	// Texture outputs
    /************************************************************/
	FragColor = vec4(current_color, 1.0);
	ExtraColor = vec4(1.0, 1.0, 0.0, 0.0); // Output extra buffers
}