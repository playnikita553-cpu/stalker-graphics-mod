/**
 * STALKER: Shadow of Chernobyl — Realistic Graphics Mod
 * Volumetric Sun Shafts (God Rays) Shader
 *
 * Screen-space radial blur for realistic volumetric light scattering.
 * Based on GPU Gems 3 approach with optimizations for X-Ray Engine.
 * Compatible with X-Ray Engine R2 renderer.
 */

#ifndef SUN_SHAFTS_H
#define SUN_SHAFTS_H

#include "common.h"

// Sun shaft parameters
uniform float4 sun_shafts_params;   // x=density, y=weight, z=decay, w=exposure
uniform float4 sun_pos_screen;      // x,y = sun screen position, z=intensity, w=unused
// sun_shafts_params defaults: (0.95, 0.25, 0.975, 0.12)

// Number of samples for radial blur
#define SHAFT_SAMPLES 64

// Generate sun shaft occlusion mask
float sun_shaft_mask(float2 tc)
{
    float depth = tex2D(s_position, tc).z;
    float sky_mask = step(depth, 0.001) + step(99.0, depth);

    // Objects closer than far plane occlude the sun
    float occlusion = saturate(sky_mask);

    return occlusion;
}

// Main sun shafts computation using radial blur
float3 compute_sun_shafts(float2 tc)
{
    float2 sun_pos = sun_pos_screen.xy;
    float intensity = sun_pos_screen.z;

    if (intensity <= 0.01)
    {
        return float3(0.0, 0.0, 0.0);
    }

    float density  = sun_shafts_params.x;
    float weight   = sun_shafts_params.y;
    float decay    = sun_shafts_params.z;
    float exposure = sun_shafts_params.w;

    // Direction from pixel to sun
    float2 delta_tc = (sun_pos - tc);
    delta_tc *= (1.0 / float(SHAFT_SAMPLES)) * density;

    float2 sample_tc = tc;
    float illumination_decay = 1.0;
    float3 shafts = float3(0.0, 0.0, 0.0);

    for (int i = 0; i < SHAFT_SAMPLES; i++)
    {
        sample_tc += delta_tc;

        // Sample occlusion mask
        float mask = sun_shaft_mask(sample_tc);

        // Accumulate
        shafts += mask * illumination_decay * weight;

        // Apply decay
        illumination_decay *= decay;
    }

    shafts *= exposure * intensity;

    // Apply warm sun color tint
    float3 sun_color = float3(1.0, 0.92, 0.75);
    shafts *= sun_color;

    return max(shafts, 0.0);
}

// Optimized version using half resolution
float3 compute_sun_shafts_half(float2 tc)
{
    float2 sun_pos = sun_pos_screen.xy;
    float intensity = sun_pos_screen.z;

    if (intensity <= 0.01)
    {
        return float3(0.0, 0.0, 0.0);
    }

    float density  = sun_shafts_params.x;
    float weight   = sun_shafts_params.y;
    float decay    = sun_shafts_params.z;
    float exposure = sun_shafts_params.w;

    float2 delta_tc = (sun_pos - tc) * (1.0 / 32.0) * density;
    float2 sample_tc = tc;
    float illumination_decay = 1.0;
    float3 shafts = float3(0.0, 0.0, 0.0);

    for (int i = 0; i < 32; i++)
    {
        sample_tc += delta_tc;
        float mask = sun_shaft_mask(sample_tc);
        shafts += mask * illumination_decay * weight;
        illumination_decay *= decay;
    }

    shafts *= exposure * intensity;
    shafts *= float3(1.0, 0.92, 0.75);

    return max(shafts, 0.0);
}

// Combine sun shafts with scene
float3 apply_sun_shafts(float3 scene, float3 shafts)
{
    // Additive blending with subtle color influence
    return scene + shafts;
}

#endif // SUN_SHAFTS_H
