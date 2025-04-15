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

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
static float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

static float2 Hammersley(uint i, uint N) {
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
static float GeometrySchlickGGX(float NdotV, float roughness) {
    float a = roughness;
    float k = (a * a) / 2.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

static float GeometrySmith(float NdotL, float NdotV, float roughness) {
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

static float3 ImportanceSampleGGX(float2 xi, float3 N, float roughness) {
    float a = roughness * roughness;
    float phi = 2 * M_PI_F * xi.x;
    float cosTheta = sqrt((1 - xi.y) / (1 + (a * a - 1) * xi.y));
    float sinTheta = sqrt(1 - cosTheta * cosTheta);
    
    float3 H(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
    
    float3 up = fabs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

static float2 IntegrateBRDF(float roughness, float NdotV) {
    float3 N(0, 0, 1);
    float3 V(sqrt(1.0 - NdotV * NdotV), 0, NdotV);
    float A = 0;
    float B = 0;

    const uint sampleCount = 1024;
    for(uint i = 0; i < sampleCount; ++i) {
        float2 x = Hammersley(i, sampleCount);
        float3 H = ImportanceSampleGGX(x, N, roughness);
        float3 L = normalize(2 * dot(V, H) * H - V);
        
        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        float VdotH = saturate(dot(V, H));
        
        if(NdotL > 0) {
            float G = GeometrySmith(NdotL, NdotV, roughness);
            float G_Vis = G * VdotH / (NdotH * NdotV);
            float Fc = powr(1 - VdotH, 5);
            A += (1 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    return float2(A, B) / float(sampleCount);
}

kernel void integrate_brdf(texture2d<half, access::write> lookup [[texture(0)]],
                           uint2 tpig [[thread_position_in_grid]])
{
    float NdotV = (tpig.x + 1) / float(lookup.get_width());
    float roughness = (tpig.y + 1) / float(lookup.get_height());
    float2 scaleAndBias = IntegrateBRDF(roughness, NdotV);
    half4 color(scaleAndBias.x, scaleAndBias.y, 0, 0);
    lookup.write(color, tpig);
}

static float2 CubeToEquirectCoords(float3 v) {
    const float2 invAtan(0.1591, 0.3183);
    float2 uv = float2(atan2(v.z, v.x), asin(v.y)) * invAtan + 0.5;
    return uv;
}

static float3 CubeDirectionFromUVAndFace(float2 uv, int face) {
    float u = uv.x;
    float v = uv.y;
    float3 dir = float3(0);
    switch (face) {
        case 0:
            dir = float3(-1,  v, -u); break; // +X
        case 1:
            dir = float3( 1,  v,  u); break; // -X
        case 2:
            dir = float3(-u, -1,  v); break; // +Y
        case 3:
            dir = float3(-u,  1, -v); break; // -Y
        case 4:
            dir = float3(-u,  v,  1); break; // +Z
        case 5:
            dir = float3( u,  v, -1); break; // -Z
    }
    
    dir = normalize(dir);
    return dir;
}

kernel void equirect_to_cube(texture2d<half, access::sample> equirectangularMap,
                             texturecube<half, access::write> cubeMap,
                             uint3 tpig [[thread_position_in_grid]])
{
    constexpr sampler sampler2d(coord::normalized, filter::linear, address::repeat);

    float cubeSize = cubeMap.get_width();
    float2 cubeUV = ((float2(tpig.xy) / cubeSize) * 2 - 1);
    int face = tpig.z;
    float3 dir = CubeDirectionFromUVAndFace(cubeUV, face);
    float2 rectUV = CubeToEquirectCoords(dir);
    half4 color = equirectangularMap.sample(sampler2d, rectUV);
    uint2 coords = tpig.xy;
    cubeMap.write(color, coords, face);
}

static float3 ComputeIrradiance(float3 N, texturecube<half, access::sample> environmentTexture) {
    constexpr sampler cubeSampler(coord::normalized, filter::linear);
    float3 irradiance = float3(0.0);
    
    float3 up(0.0, 1.0, 0.0);
    float3 right = cross(up, N);
    up = cross(N, right);

    float sampleDelta = 0.025;
    float sampleCount = 0;
    for (float phi = 0.0; phi < M_PI_F * 2; phi += sampleDelta) {
        for (float theta = 0.0; theta < M_PI_F * 0.5; theta += sampleDelta) {
            // spherical to cartesian (in tangent space)
            float3 tangentSample(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 dir = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += float3(environmentTexture.sample(cubeSampler, dir).rgb) * cos(theta) * sin(theta);
            sampleCount += 1;
        }
    }

    irradiance = M_PI_F * irradiance * (1.0 / sampleCount);
    return irradiance;
}

kernel void compute_irradiance(texturecube<half, access::sample> environmentMap,
                               texturecube<half, access::write> irradianceMap,
                               uint3 tpig [[thread_position_in_grid]])
{
    float cubeSize = irradianceMap.get_width();
    float2 cubeUV = ((float2(tpig.xy) / cubeSize) * 2 - 1);
    int face = tpig.z;
    float3 dir = CubeDirectionFromUVAndFace(cubeUV, face);
    dir *= float3(-1, -1, 1);
    float3 irrad = ComputeIrradiance(dir, environmentMap);
    uint2 coords = tpig.xy;
    irradianceMap.write(half4(half3(irrad), 1), coords, face);
}

static float3 PrefilterEnvMap(float roughness, float3 R, texturecube<half, access::sample> environmentTexture) {
    constexpr sampler cubeSampler(coord::normalized, filter::linear, mip_filter::linear);
    
    float3 N = R;
    float3 V = R;
    
    float3 prefilteredColor(0);
    float totalWeight = 0;
    float resolution = environmentTexture.get_width();
    float saTexel  = 4.0 * M_PI_F / (6.0 * resolution * resolution);

    const uint sampleCount = 512;
    for(uint i = 0; i < sampleCount; ++i) {
        float2 xi = Hammersley(i, sampleCount);
        float3 H = ImportanceSampleGGX(xi, N, roughness);
        float3 L = normalize(2 * dot(V, H) * H - V);
        float NdotL = saturate(dot(N, L));
        if(NdotL > 0) {
            float NdotH = saturate(dot(N, H));
            float HdotV = saturate(dot(H, V));
            float D   = GeometrySchlickGGX(NdotH, roughness);
            float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001;
            float saSample = 1.0 / (float(sampleCount) * pdf + 0.0001);
            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
            prefilteredColor += NdotL * float3(environmentTexture.sample(cubeSampler, L, level(mipLevel)).rgb);
            totalWeight += NdotL;
        }
    }
    return prefilteredColor / totalWeight;
}

kernel void compute_prefiltered_specular(texturecube<half, access::sample> environmentMap,
                                         texturecube<half, access::write> specularMap,
                                         constant float &roughness [[buffer(0)]],
                                         uint3 tpig [[thread_position_in_grid]])
{
    float cubeSize = specularMap.get_width();
    float2 cubeUV = ((float2(tpig.xy) / cubeSize) * 2 - 1);
    int face = tpig.z;
    float3 dir = CubeDirectionFromUVAndFace(cubeUV, face);
    dir *= float3(-1, -1, 1);
    float3 irrad = PrefilterEnvMap(roughness, dir, environmentMap);
    uint2 coords = tpig.xy;
    specularMap.write(half4(half3(irrad), 1), coords, face);
}
