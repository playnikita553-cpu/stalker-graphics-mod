/**
 * STALKER: Shadow of Chernobyl — Realistic Graphics Mod
 * Color Grading / Post-Processing Shader
 *
 * Cinematic color correction with tone mapping, contrast,
 * saturation control, and Zone-specific color palette.
 * Compatible with X-Ray Engine R2 renderer.
 */

#ifndef COLOR_GRADING_H
#define COLOR_GRADING_H

#include "common.h"

// Color grading parameters
uniform float4 cg_params;      // x=contrast, y=saturation, z=brightness, w=gamma
uniform float4 cg_color_tint;  // xyz=tint_color, w=tint_strength
uniform float4 cg_shadows;     // xyz=shadow_color, w=shadow_strength
uniform float4 cg_highlights;  // xyz=highlight_color, w=highlight_strength
// cg_params defaults: (1.12, 0.92, 1.0, 1.05)
// cg_color_tint defaults: (0.95, 0.98, 1.05, 0.15) — slight cold blue Zone tint
// cg_shadows defaults: (0.15, 0.18, 0.25, 0.4) — blue shadows
// cg_highlights defaults: (1.0, 0.95, 0.85, 0.25) — warm highlights

// Luminance calculation
float luma(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

// Filmic tone mapping (Uncharted 2 style)
float3 tonemap_uncharted2(float3 x)
{
    float A = 0.15;  // Shoulder Strength
    float B = 0.50;  // Linear Strength
    float C = 0.10;  // Linear Angle
    float D = 0.20;  // Toe Strength
    float E = 0.02;  // Toe Numerator
    float F = 0.30;  // Toe Denominator

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

// ACES filmic tone mapping
float3 tonemap_aces(float3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// Reinhard tone mapping with white point
float3 tonemap_reinhard(float3 color, float white_point)
{
    float l = luma(color);
    float mapped = (l * (1.0 + l / (white_point * white_point))) / (1.0 + l);
    return color * (mapped / max(l, 0.001));
}

// Lift-Gamma-Gain color correction
float3 lift_gamma_gain(float3 color, float3 lift, float3 gamma, float3 gain)
{
    float3 result = gain * (lift * (1.0 - color) + pow(max(color, 0.0), gamma));
    return saturate(result);
}

// Zone atmosphere color grading
float3 zone_color_grade(float3 color)
{
    float contrast   = cg_params.x;
    float saturation = cg_params.y;
    float brightness = cg_params.z;
    float gamma      = cg_params.w;

    // Apply brightness
    color *= brightness;

    // Apply contrast (around midpoint 0.5)
    color = ((color - 0.5) * contrast) + 0.5;

    // Apply saturation
    float grey = luma(color);
    color = lerp(float3(grey, grey, grey), color, saturation);

    // Apply gamma correction
    color = pow(max(color, 0.0), float3(1.0 / gamma, 1.0 / gamma, 1.0 / gamma));

    // Shadow/highlight color split
    float luminance = luma(color);
    float shadow_mask = 1.0 - smoothstep(0.0, 0.5, luminance);
    float highlight_mask = smoothstep(0.5, 1.0, luminance);

    // Tint shadows (blue/cold for Zone atmosphere)
    color = lerp(color, color * cg_shadows.rgb / max(luma(cg_shadows.rgb), 0.001),
                 shadow_mask * cg_shadows.w);

    // Tint highlights (warm)
    color = lerp(color, color * cg_highlights.rgb / max(luma(cg_highlights.rgb), 0.001),
                 highlight_mask * cg_highlights.w);

    // Apply overall color tint
    color = lerp(color, color * cg_color_tint.rgb, cg_color_tint.w);

    return saturate(color);
}

// Vignette effect
float vignette(float2 tc, float strength, float radius)
{
    float2 center = tc - 0.5;
    float dist = length(center);
    float vign = smoothstep(radius, radius - 0.45, dist);
    return lerp(1.0, vign, strength);
}

// Film grain (subtle)
float film_grain(float2 tc, float time, float strength)
{
    float noise = frac(sin(dot(tc * time, float2(12.9898, 78.233))) * 43758.5453);
    return lerp(1.0, 0.5 + noise, strength);
}

// Complete post-processing pipeline
float3 apply_color_grading(float3 color, float2 tc)
{
    // Tone mapping (using ACES for cinematic look)
    color = tonemap_aces(color * 1.2);

    // Zone-specific color grading
    color = zone_color_grade(color);

    // Subtle vignette
    float vign = vignette(tc, 0.35, 0.85);
    color *= vign;

    return color;
}

#endif // COLOR_GRADING_H
