/**
 * STALKER: Shadow of Chernobyl — Realistic Graphics Mod
 * Enhanced Shadows Shader
 *
 * Soft shadow filtering with PCF and contact hardening.
 * Improved shadow quality at distance.
 * Compatible with X-Ray Engine R2 renderer.
 */

#ifndef ENHANCED_SHADOWS_H
#define ENHANCED_SHADOWS_H

#include "common.h"

// Shadow parameters
uniform float4 shadow_params;   // x=bias, y=softness, z=distance_fade, w=intensity
// shadow_params defaults: (-0.0001, 2.5, 1.0, 0.85)

// Poisson disk for shadow sampling
static const float2 shadow_poisson[16] =
{
    float2(-0.94201624, -0.39906216),
    float2( 0.94558609, -0.76890725),
    float2(-0.09418410, -0.92938870),
    float2( 0.34495938,  0.29387760),
    float2(-0.91588581,  0.45771432),
    float2(-0.81544232, -0.87912464),
    float2(-0.38277543,  0.27676845),
    float2( 0.97484398,  0.75648379),
    float2( 0.44323325, -0.97511554),
    float2( 0.53742981, -0.47373420),
    float2(-0.26496911, -0.41893023),
    float2( 0.79197514,  0.19090188),
    float2(-0.24188840,  0.99706507),
    float2(-0.81409955,  0.91437590),
    float2( 0.19984126,  0.78641367),
    float2( 0.14383161, -0.14100790)
};

// PCF shadow sampling (Percentage Closer Filtering)
float pcf_shadow_4tap(float4 shadow_tc, float2 texel_size)
{
    float shadow = 0.0;
    float bias = shadow_params.x;

    float2 offsets[4] =
    {
        float2(-0.5, -0.5),
        float2( 0.5, -0.5),
        float2(-0.5,  0.5),
        float2( 0.5,  0.5)
    };

    for (int i = 0; i < 4; i++)
    {
        float2 tc = shadow_tc.xy + offsets[i] * texel_size;
        float depth = tex2D(s_smap, tc).r;
        shadow += (shadow_tc.z + bias < depth) ? 1.0 : 0.0;
    }

    return shadow * 0.25;
}

// High quality PCF with Poisson disk
float pcf_shadow_16tap(float4 shadow_tc, float2 texel_size, float softness)
{
    float shadow = 0.0;
    float bias = shadow_params.x;
    float spread = softness * shadow_params.y;

    for (int i = 0; i < 16; i++)
    {
        float2 tc = shadow_tc.xy + shadow_poisson[i] * texel_size * spread;
        float depth = tex2D(s_smap, tc).r;
        shadow += (shadow_tc.z + bias < depth) ? 1.0 : 0.0;
    }

    return shadow * (1.0 / 16.0);
}

// Contact hardening shadows (PCSS-like)
float contact_hardening_shadow(float4 shadow_tc, float2 texel_size, float light_size)
{
    float bias = shadow_params.x;

    // Step 1: Find average blocker depth
    float blocker_sum = 0.0;
    float blocker_count = 0.0;
    float search_radius = light_size * 3.0;

    for (int i = 0; i < 16; i++)
    {
        float2 tc = shadow_tc.xy + shadow_poisson[i] * texel_size * search_radius;
        float depth = tex2D(s_smap, tc).r;

        if (depth < shadow_tc.z + bias)
        {
            blocker_sum += depth;
            blocker_count += 1.0;
        }
    }

    if (blocker_count < 1.0)
    {
        return 1.0; // No blockers, fully lit
    }

    float avg_blocker_depth = blocker_sum / blocker_count;

    // Step 2: Calculate penumbra size
    float penumbra = (shadow_tc.z - avg_blocker_depth) * light_size / avg_blocker_depth;
    penumbra = max(penumbra, 0.5);

    // Step 3: PCF with variable kernel
    return pcf_shadow_16tap(shadow_tc, texel_size, penumbra);
}

// Distance-based shadow fade
float shadow_distance_fade(float depth, float max_distance)
{
    float fade_start = max_distance * 0.7;
    return 1.0 - smoothstep(fade_start, max_distance, depth);
}

// Apply shadow with intensity control
float apply_shadow(float shadow, float intensity)
{
    // Soften shadow darkness (never fully black)
    float min_shadow = 1.0 - intensity;
    return lerp(min_shadow, 1.0, shadow);
}

#endif // ENHANCED_SHADOWS_H
