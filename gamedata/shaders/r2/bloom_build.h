/**
 * STALKER: Shadow of Chernobyl — Realistic Graphics Mod
 * Enhanced Bloom Build Shader
 *
 * Extracts bright areas from the scene for bloom processing.
 * Uses improved threshold and knee for natural-looking glow.
 * Compatible with X-Ray Engine R2 renderer.
 */

#ifndef BLOOM_BUILD_H
#define BLOOM_BUILD_H

#include "common.h"

// Bloom parameters
uniform float4 bloom_params;    // x=threshold, y=knee, z=intensity, w=unused
// bloom_params defaults: (0.75, 0.5, 0.65, 0)

struct v2p_bloom
{
    float4 hpos     : POSITION;
    float2 tc0      : TEXCOORD0;
    float4 tc1      : TEXCOORD1;
    float4 tc2      : TEXCOORD2;
    float4 tc3      : TEXCOORD3;
};

// Soft threshold function for smooth bloom transition
float3 soft_threshold(float3 color, float threshold, float knee)
{
    float brightness = max(color.r, max(color.g, color.b));
    float soft = brightness - threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee + 0.00001);
    float contribution = max(soft, brightness - threshold);
    contribution /= max(brightness, 0.00001);
    return color * max(contribution, 0.0);
}

// Luminance calculation using perceptual weights
float luminance(float3 color)
{
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

// Prefilter pass: extract bright areas
float4 bloom_prefilter(float2 tc)
{
    float3 color = tex2D(s_image, tc).rgb;

    float threshold = bloom_params.x;
    float knee = bloom_params.y * threshold;

    // Apply soft threshold
    float3 bloom = soft_threshold(color, threshold, knee);

    // Prevent fireflies by clamping extremely bright pixels
    float lum = luminance(bloom);
    if (lum > 0.0)
    {
        bloom *= 1.0 / (1.0 + lum);
    }

    return float4(bloom, 1.0);
}

// Downsample with 13-tap filter for better quality
float3 downsample_13tap(float2 tc, float2 texel_size)
{
    float3 a = tex2D(s_image, tc + texel_size * float2(-1.0, -1.0)).rgb;
    float3 b = tex2D(s_image, tc + texel_size * float2( 0.0, -1.0)).rgb;
    float3 c = tex2D(s_image, tc + texel_size * float2( 1.0, -1.0)).rgb;
    float3 d = tex2D(s_image, tc + texel_size * float2(-0.5, -0.5)).rgb;
    float3 e = tex2D(s_image, tc + texel_size * float2( 0.5, -0.5)).rgb;
    float3 f = tex2D(s_image, tc + texel_size * float2(-1.0,  0.0)).rgb;
    float3 g = tex2D(s_image, tc).rgb;
    float3 h = tex2D(s_image, tc + texel_size * float2( 1.0,  0.0)).rgb;
    float3 i = tex2D(s_image, tc + texel_size * float2(-0.5,  0.5)).rgb;
    float3 j = tex2D(s_image, tc + texel_size * float2( 0.5,  0.5)).rgb;
    float3 k = tex2D(s_image, tc + texel_size * float2(-1.0,  1.0)).rgb;
    float3 l = tex2D(s_image, tc + texel_size * float2( 0.0,  1.0)).rgb;
    float3 m = tex2D(s_image, tc + texel_size * float2( 1.0,  1.0)).rgb;

    float3 result = float3(0.0, 0.0, 0.0);
    // Center cross (weight = 0.5)
    result += (d + e + i + j) * 0.125;
    // Corners (weight = 0.125 each group)
    result += (a + b + f + g) * 0.03125;
    result += (b + c + g + h) * 0.03125;
    result += (f + g + k + l) * 0.03125;
    result += (g + h + l + m) * 0.03125;

    return result;
}

#endif // BLOOM_BUILD_H
