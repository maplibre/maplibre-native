//
//  Copyright (c) 2018 Warren Moore. All rights reserved.
//
//  Permission to use, copy, modify, and distribute this software for any
//  purpose with or without fee is hereby granted, provided that the above
//  copyright notice and this permission notice appear in all copies.
//
//  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
//  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
//  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
//  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
//  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
//  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#include <metal_stdlib>
using namespace metal;

constexpr sampler linearSampler(coord::normalized, min_filter::linear, mag_filter::linear);
constexpr sampler nearestSampler(coord::normalized, min_filter::nearest, mag_filter::nearest);

struct VertexOut {
    float4 position [[position]];
    float2 texCoords;
};

vertex VertexOut quad_vertex_main(constant packed_float4 *vertices [[buffer(0)]],
                                  uint vid [[vertex_id]])
{
    VertexOut out;
    float4 in = vertices[vid];
    out.position = float4(in.xy, 0, 1);
    out.texCoords = in.zw;
    return out;
}

typedef VertexOut FragmentIn;

static half3 reinhardToneMapping(half3 color) {
    half exposure = 1.5;
    color *= exposure / (1 + color / exposure);
    return color;
}

fragment half4 tonemap_fragment_main(FragmentIn in [[stage_in]],
                                     texture2d<half, access::sample> sourceTexture [[texture(0)]])
{
    // Original, pre-alpha
//    half3 color = sourceTexture.sample(linearSampler, in.texCoords).rgb;
//    return half4(reinhardToneMapping(color), 1);
    
    half4 color = sourceTexture.sample(linearSampler, in.texCoords);
    return half4(reinhardToneMapping(color.rgb), color.a);

}


fragment half4 bloom_threshold_fragment_main(FragmentIn in [[stage_in]],
                                             texture2d<half, access::sample> sourceTexture [[texture(0)]])
{
    const float bloomThreshold = 0.88;
    const float bloomIntensity = 2;
    half4 color = sourceTexture.sample(linearSampler, in.texCoords);
    half luma = dot(color.rgb, half3(0.2126, 0.7152, 0.0722));
    return (luma > bloomThreshold) ? color * bloomIntensity : half4(0, 0, 0, 1);
}

fragment half4 blur_horizontal7_fragment_main(FragmentIn in [[stage_in]],
                                              texture2d<half, access::sample> sourceTexture [[texture(0)]])
{
    float weights[]{ 0.134032, 0.126854, 0.107545, 0.08167, 0.055555, 0.033851, 0.018476, 0.009033 };
    float offset = 1.0 / sourceTexture.get_width();
    half4 color(0);
    color += weights[7] * sourceTexture.sample(nearestSampler, in.texCoords - float2(offset * 7, 0));
    color += weights[6] * sourceTexture.sample(nearestSampler, in.texCoords - float2(offset * 6, 0));
    color += weights[5] * sourceTexture.sample(nearestSampler, in.texCoords - float2(offset * 5, 0));
    color += weights[4] * sourceTexture.sample(nearestSampler, in.texCoords - float2(offset * 4, 0));
    color += weights[3] * sourceTexture.sample(nearestSampler, in.texCoords - float2(offset * 3, 0));
    color += weights[2] * sourceTexture.sample(nearestSampler, in.texCoords - float2(offset * 2, 0));
    color += weights[1] * sourceTexture.sample(nearestSampler, in.texCoords - float2(offset * 1, 0));
    color += weights[0] * sourceTexture.sample(nearestSampler, in.texCoords + float2(offset * 0, 0));
    color += weights[1] * sourceTexture.sample(nearestSampler, in.texCoords + float2(offset * 1, 0));
    color += weights[2] * sourceTexture.sample(nearestSampler, in.texCoords + float2(offset * 2, 0));
    color += weights[3] * sourceTexture.sample(nearestSampler, in.texCoords + float2(offset * 3, 0));
    color += weights[4] * sourceTexture.sample(nearestSampler, in.texCoords + float2(offset * 4, 0));
    color += weights[5] * sourceTexture.sample(nearestSampler, in.texCoords + float2(offset * 5, 0));
    color += weights[6] * sourceTexture.sample(nearestSampler, in.texCoords + float2(offset * 6, 0));
    color += weights[7] * sourceTexture.sample(nearestSampler, in.texCoords + float2(offset * 7, 0));
    return color;
}

fragment half4 blur_vertical7_fragment_main(FragmentIn in [[stage_in]],
                                            texture2d<half, access::sample> sourceTexture [[texture(0)]])
{
    float weights[]{ 0.134032, 0.126854, 0.107545, 0.08167, 0.055555, 0.033851, 0.018476, 0.009033 };
    float offset = 1.0 / sourceTexture.get_height();
    half4 color(0);
    color += weights[7] * sourceTexture.sample(nearestSampler, in.texCoords - float2(0, offset * 7));
    color += weights[6] * sourceTexture.sample(nearestSampler, in.texCoords - float2(0, offset * 6));
    color += weights[5] * sourceTexture.sample(nearestSampler, in.texCoords - float2(0, offset * 5));
    color += weights[4] * sourceTexture.sample(nearestSampler, in.texCoords - float2(0, offset * 4));
    color += weights[3] * sourceTexture.sample(nearestSampler, in.texCoords - float2(0, offset * 3));
    color += weights[2] * sourceTexture.sample(nearestSampler, in.texCoords - float2(0, offset * 2));
    color += weights[1] * sourceTexture.sample(nearestSampler, in.texCoords - float2(0, offset * 1));
    color += weights[0] * sourceTexture.sample(nearestSampler, in.texCoords + float2(0, offset * 0));
    color += weights[1] * sourceTexture.sample(nearestSampler, in.texCoords + float2(0, offset * 1));
    color += weights[2] * sourceTexture.sample(nearestSampler, in.texCoords + float2(0, offset * 2));
    color += weights[3] * sourceTexture.sample(nearestSampler, in.texCoords + float2(0, offset * 3));
    color += weights[4] * sourceTexture.sample(nearestSampler, in.texCoords + float2(0, offset * 4));
    color += weights[5] * sourceTexture.sample(nearestSampler, in.texCoords + float2(0, offset * 5));
    color += weights[6] * sourceTexture.sample(nearestSampler, in.texCoords + float2(0, offset * 6));
    color += weights[7] * sourceTexture.sample(nearestSampler, in.texCoords + float2(0, offset * 7));
    return color;
}

fragment half4 additive_blend_fragment_main(FragmentIn in [[stage_in]],
                                            texture2d<half, access::sample> sourceTexture [[texture(0)]])
{
    half4 color = sourceTexture.sample(linearSampler, in.texCoords);
    return color;
}

