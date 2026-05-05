/**
 * STALKER: Shadow of Chernobyl — Realistic Graphics Mod
 * Depth of Field (DOF) Shader
 *
 * Cinematic depth-of-field with smooth bokeh approximation.
 * Supports near and far blur with adjustable focus distance.
 * Compatible with X-Ray Engine R2 renderer.
 */

#ifndef DOF_H
#define DOF_H

#include "common.h"

// DOF parameters
uniform float4 dof_params;      // x=near_start, y=near_end, z=far_start, w=far_end
uniform float4 dof_kernel;      // x=max_coc_radius, y=bokeh_intensity, z=unused, w=unused
// dof_params defaults: (0.0, 0.2, 15.0, 80.0)
// dof_kernel defaults: (5.0, 1.2, 0, 0)

// Poisson disk sampling pattern for bokeh-like blur
static const float2 poisson_disk[12] =
{
    float2(-0.326212, -0.405805),
    float2(-0.840144, -0.073580),
    float2(-0.695914,  0.457137),
    float2(-0.203345,  0.620716),
    float2( 0.962340, -0.194983),
    float2( 0.473434, -0.480026),
    float2( 0.519456,  0.767022),
    float2( 0.185461, -0.893124),
    float2( 0.507431,  0.064425),
    float2( 0.896420,  0.412458),
    float2(-0.321940, -0.932615),
    float2(-0.791559, -0.597705)
};

// Calculate Circle of Confusion (CoC) from depth
float calculate_coc(float depth)
{
    float near_start = dof_params.x;
    float near_end   = dof_params.y;
    float far_start  = dof_params.z;
    float far_end    = dof_params.w;

    float coc = 0.0;

    // Near blur
    if (depth < near_end)
    {
        coc = -smoothstep(near_end, near_start, depth);
    }
    // Far blur
    else if (depth > far_start)
    {
        coc = smoothstep(far_start, far_end, depth);
    }

    return coc;
}

// Enhanced DOF blur using Poisson disk sampling
float3 dof_blur(float2 tc, float2 texel_size, float coc)
{
    float max_radius = dof_kernel.x;
    float radius = abs(coc) * max_radius;

    if (radius < 0.5)
    {
        return tex2D(s_image, tc).rgb;
    }

    float3 color = float3(0.0, 0.0, 0.0);
    float total_weight = 0.0;

    // Center sample
    float3 center_color = tex2D(s_image, tc).rgb;
    float center_depth = tex2D(s_position, tc).z;
    float center_coc = calculate_coc(center_depth);

    for (int i = 0; i < 12; i++)
    {
        float2 offset = poisson_disk[i] * radius * texel_size;
        float2 sample_tc = tc + offset;

        float3 sample_color = tex2D(s_image, sample_tc).rgb;
        float sample_depth = tex2D(s_position, sample_tc).z;
        float sample_coc = calculate_coc(sample_depth);

        // Prevent background bleeding onto sharp foreground
        float weight = 1.0;
        if (sample_depth > center_depth && abs(center_coc) < abs(sample_coc))
        {
            weight = smoothstep(0.0, abs(center_coc), abs(sample_coc) * 0.5);
        }

        // Bokeh brightness boost for bright highlights
        float brightness = dot(sample_color, float3(0.2126, 0.7152, 0.0722));
        float bokeh_weight = 1.0 + smoothstep(0.5, 1.0, brightness) * (dof_kernel.y - 1.0);

        weight *= bokeh_weight;
        color += sample_color * weight;
        total_weight += weight;
    }

    color /= max(total_weight, 0.001);

    // Blend based on CoC
    float blend = smoothstep(0.0, 2.0, radius);
    return lerp(center_color, color, blend);
}

// Simplified DOF for performance mode
float3 dof_simple(float2 tc, float2 texel_size, float coc)
{
    float radius = abs(coc) * dof_kernel.x;

    if (radius < 0.5)
    {
        return tex2D(s_image, tc).rgb;
    }

    float3 color = tex2D(s_image, tc).rgb;
    float total = 1.0;

    // 5-tap cross filter
    float2 offsets[4] =
    {
        float2( 1.0,  0.0),
        float2(-1.0,  0.0),
        float2( 0.0,  1.0),
        float2( 0.0, -1.0)
    };

    for (int i = 0; i < 4; i++)
    {
        float2 sample_tc = tc + offsets[i] * radius * texel_size;
        color += tex2D(s_image, sample_tc).rgb;
        total += 1.0;
    }

    return color / total;
}

#endif // DOF_H
