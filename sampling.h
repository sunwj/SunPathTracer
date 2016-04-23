//
// Created by 孙万捷 on 16/3/21.
//

#ifndef SUNPATHTRACER_SAMPLING_H
#define SUNPATHTRACER_SAMPLING_H

#include <curand.h>
#include <curand_kernel.h>

#include "helper_math.h"
#include "cuda_onb.h"

// return r and theta in polar coordinate
__inline__ __device__ float2 uniform_sample_unit_disk(curandState& rng)
{
    float r = sqrtf(curand_uniform(&rng));
    float theta = curand_uniform(&rng) * 2.f * M_PI;

    return make_float2(r, theta);
}

// return x and y in cartesian coordinate
__inline__ __device__ float2 uniform_sample_disk(curandState& rng, float r)
{
    return make_float2(cosf(2.f * M_PI * curand_uniform(&rng)), sinf(sqrtf(curand_uniform(&rng)))) * r;
}

// return direction in cartesian space
__inline__ __device__ float3 uniform_sample_hemisphere(curandState& rng, const float3& n)
{
    cudaONB onb(n);
    float phi = 2.f * M_PI * curand_uniform(&rng);

    float cosTheta = curand_uniform(&rng);
    float sinTheta = sqrtf(fmaxf(0.f, 1.f - cosTheta * cosTheta));

    return sinTheta * cosf(phi) * onb.u + sinTheta * sinf(phi) * onb.v + cosTheta * onb.w;
}

// return direction in cartesian space
__inline__ __device__ float3 cosine_weightd_sample_hemisphere(curandState& rng, const float3& n)
{
    cudaONB onb(n);
    float phi = 2.f * M_PI * curand_uniform(&rng);

    float sinTheta = sqrtf(curand_uniform(&rng));
    float cosTheta = sqrtf(fmaxf(0.f, 1.f - sinTheta * sinTheta));

    return sinTheta * cosf(phi) * onb.u + sinTheta * sinf(phi) * onb.v + cosTheta * onb.w;
}

__inline__ __device__ float3 sample_phong(curandState& rng, float roughness, const float3& r)
{
    cudaONB onb(r);
    float phi = 2.f * M_PI * curand_uniform(&rng);
    float cosTheta = powf(1.f - curand_uniform(&rng), 1.f / (roughness + 1.f));
    float sinTheta = sqrtf(fmaxf(0.f, 1.f - cosTheta * cosTheta));

    return normalize(sinTheta * cosf(phi) * onb.u + sinTheta * sinf(phi) * onb.v + cosTheta * onb.w);
}

#endif //SUNPATHTRACER_SAMPLING_H
