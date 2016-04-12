//
// Created by 孙万捷 on 16/4/12.
//

#ifndef SUNPATHTRACER_TONEMAPPING_H
#define SUNPATHTRACER_TONEMAPPING_H

#include <cuda_runtime.h>
#include "helper_math.h"

__inline__ __device__ float3 reinhard_tone_mapping(const float3& L, float exposure, float gamma = 1.f / 2.2f)
{
    //hardcoded exposure adjustment
    float3 l = L * 16.f;
    l.x = 1.f - expf(-l.x * exposure);
    l.y = 1.f - expf(-l.y * exposure);
    l.z = 1.f - expf(-l.z * exposure);

    float invGamma = 1.f / gamma;
    l.x = powf(l.x, invGamma);
    l.y = powf(l.y, invGamma);
    l.z = powf(l.z, invGamma);

    return l;
}

#endif //SUNPATHTRACER_TONEMAPPING_H
