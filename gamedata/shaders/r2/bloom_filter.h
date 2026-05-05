/**
 * STALKER: Shadow of Chernobyl — Realistic Graphics Mod
 * Enhanced Bloom Filter Shader
 *
 * Applies Gaussian blur passes for smooth, cinematic bloom.
 * Uses dual-pass Kawase blur for performance.
 * Compatible with X-Ray Engine R2 renderer.
 */

#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include "common.h"

// Upsample with tent filter (3x3) for smooth combination
float3 upsample_tent(float2 tc, float2 texel_size)
{
    float4 d = texel_size.xyxy * float4(1.0, 1.0, -1.0, 0.0);

    float3 s;
    s  = tex2D(s_bloom, tc - d.xy).rgb;
    s += tex2D(s_bloom, tc - d.wy).rgb * 2.0;
    s += tex2D(s_bloom, tc - d.zy).rgb;

    s += tex2D(s_bloom, tc + d.zw).rgb * 2.0;
    s += tex2D(s_bloom, tc        ).rgb * 4.0;
    s += tex2D(s_bloom, tc + d.xw).rgb * 2.0;

    s += tex2D(s_bloom, tc + d.zy).rgb;
    s += tex2D(s_bloom, tc + d.wy).rgb * 2.0;
    s += tex2D(s_bloom, tc + d.xy).rgb;

    return s * (1.0 / 16.0);
}

// 9-tap Gaussian blur (horizontal)
float3 gaussian_h(float2 tc, float2 texel_size)
{
    float offsets[4] = { 0.0, 1.3846153846, 3.2307692308, 5.1538461538 };
    float weights[4] = { 0.22702702703, 0.31621621622, 0.07027027027, 0.00302702703 };

    float3 result = tex2D(s_bloom, tc).rgb * weights[0];

    for (int i = 1; i < 4; i++)
    {
        float2 offset = float2(texel_size.x * offsets[i], 0.0);
        result += tex2D(s_bloom, tc + offset).rgb * weights[i];
        result += tex2D(s_bloom, tc - offset).rgb * weights[i];
    }

    return result;
}

// 9-tap Gaussian blur (vertical)
float3 gaussian_v(float2 tc, float2 texel_size)
{
    float offsets[4] = { 0.0, 1.3846153846, 3.2307692308, 5.1538461538 };
    float weights[4] = { 0.22702702703, 0.31621621622, 0.07027027027, 0.00302702703 };

    float3 result = tex2D(s_bloom, tc).rgb * weights[0];

    for (int i = 1; i < 4; i++)
    {
        float2 offset = float2(0.0, texel_size.y * offsets[i]);
        result += tex2D(s_bloom, tc + offset).rgb * weights[i];
        result += tex2D(s_bloom, tc - offset).rgb * weights[i];
    }

    return result;
}

// Final bloom composite
float3 bloom_composite(float3 scene, float3 bloom, float intensity)
{
    // Apply bloom with intensity control
    float3 result = scene + bloom * intensity;

    // Subtle lens dirt effect (simulated)
    float bloom_lum = dot(bloom, float3(0.2126, 0.7152, 0.0722));
    float dirt_factor = smoothstep(0.3, 1.0, bloom_lum) * 0.15;
    result += bloom * dirt_factor;

    return result;
}

#endif // BLOOM_FILTER_H
