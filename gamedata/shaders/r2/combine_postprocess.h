/**
 * STALKER: Shadow of Chernobyl — Realistic Graphics Mod
 * Combined Post-Processing Pipeline
 *
 * Main combine shader that orchestrates all post-processing effects:
 * bloom, DOF, sun shafts, color grading, and final output.
 * Compatible with X-Ray Engine R2 renderer.
 */

#ifndef COMBINE_POSTPROCESS_H
#define COMBINE_POSTPROCESS_H

#include "common.h"
#include "bloom_build.h"
#include "bloom_filter.h"
#include "dof.h"
#include "sun_shafts.h"
#include "color_grading.h"

// Master control parameters
uniform float4 pp_flags;       // x=enable_bloom, y=enable_dof, z=enable_shafts, w=enable_grading
uniform float4 pp_intensity;   // x=bloom_strength, y=dof_strength, z=shafts_strength, w=grading_strength
// pp_flags defaults: (1.0, 1.0, 1.0, 1.0)
// pp_intensity defaults: (0.65, 1.0, 0.85, 1.0)

// Main post-processing combine
float4 combine_final(float2 tc, float2 texel_size)
{
    // Read scene color
    float3 scene = tex2D(s_image, tc).rgb;
    float depth = tex2D(s_position, tc).z;

    // ---- BLOOM ----
    if (pp_flags.x > 0.5)
    {
        float3 bloom = tex2D(s_bloom, tc).rgb;
        scene = bloom_composite(scene, bloom, pp_intensity.x);
    }

    // ---- DEPTH OF FIELD ----
    if (pp_flags.y > 0.5)
    {
        float coc = calculate_coc(depth);
        if (abs(coc) > 0.01)
        {
            float3 blurred = dof_blur(tc, texel_size, coc);
            scene = lerp(scene, blurred, abs(coc) * pp_intensity.y);
        }
    }

    // ---- SUN SHAFTS ----
    if (pp_flags.z > 0.5)
    {
        float3 shafts = compute_sun_shafts(tc);
        scene = apply_sun_shafts(scene, shafts * pp_intensity.z);
    }

    // ---- COLOR GRADING ----
    if (pp_flags.w > 0.5)
    {
        scene = apply_color_grading(scene, tc);
    }

    return float4(scene, 1.0);
}

#endif // COMBINE_POSTPROCESS_H
