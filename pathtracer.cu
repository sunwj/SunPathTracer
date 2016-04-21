#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include "helper_math.h"
#include "cuda_shape.h"
#include "cuda_camera.h"
#include "cuda_scene.h"
#include "tonemapping.h"
#include "sampling.h"
#include "render_parameters.h"
#include "kernel_globals.h"

auto constexpr WIDTH = 640;
auto constexpr HEIGHT = 480;

__global__ void testSimpleScene(uchar4* img, cudaScene scene, RenderParameters params, unsigned int hashed_N)
{
    unsigned int idx = blockDim.x * blockIdx.x + threadIdx.x;
    unsigned int idy = blockDim.y * blockIdx.y + threadIdx.y;
    unsigned int offset = idy * scene.camera.imageW + idx;
    img[offset] = IMG_BLACK;

    curandState rng;
    curand_init(hashed_N + offset, 0, 0, &rng);
    cudaRay ray;
    scene.camera.GenerateRay(idx, idy, rng, &ray);

    float3 L = make_float3(0.f, 0.f, 0.f);
    float3 T = make_float3(1.f, 1.f, 1.f);

    HitInfo hi;
    for(auto k = 0; k < 5; ++k)
    {
        if(!scene_intersect(scene, ray, hi)) break;
        L += T * scene.materials[hi.matID].emition;

        if(scene.materials[hi.matID].bsdf_type == BSDF_DIFFUSE)
        {
            ray.orig = hi.pt;
            ray.dir = cosine_weightd_sample_hemisphere(rng, hi.normal);

            T *= scene.materials[hi.matID].albedo;
        }

        if(scene.materials[hi.matID].bsdf_type == BSDF_GLASS)
        {
            float eta = scene.materials[hi.matID].ior;
            if(dot(hi.normal, ray.dir) > 0.f)
            {
                eta = 1.f / eta;
                hi.normal = -hi.normal;
            }
            eta = 1.f / eta;
            float cosin = -dot(hi.normal, ray.dir);
            float cost2 = 1.f - eta * eta * (1.f - cosin * cosin);

            if(cost2 < 0.f)
            {
                T *= scene.materials[hi.matID].albedo;
                ray.dir = reflect(ray.dir, hi.normal);
            }
            else
            {
                float3 tdir = eta * ray.dir + hi.normal * (eta * cosin - sqrtf(cost2));
                tdir = normalize(tdir);

                float n1 = (cosin < 0.f) ? 1.f : scene.materials[hi.matID].ior;
                float n2 = (cosin < 0.f) ? scene.materials[hi.matID].ior : 1.f;
                float R0 = (n1 - n2) * (n1 - n2) / ((n1 + n2) * (n1 + n2));
                float c = cosin;
                float Pr = R0 + (1.f - R0) * c * c * c * c * c;
                float Pt = 1.f - Pr;
                float P = 0.25f + 0.5f * Pr;

                if(curand_uniform(&rng) < P)
                {
                    T *= scene.materials[hi.matID].albedo;
                    T *= (Pr / P);
                    ray.dir = reflect(ray.dir, hi.normal);
                }
                else
                {
                    T *= scene.materials[hi.matID].albedo;
                    T *= (Pt / (1.f - P));
                    ray.dir = tdir;
                }
            }

            ray.orig = hi.pt;
        }
    }

    running_estimate(params.hdr_buffer[offset], L, params.iteration_count);
    L = reinhard_tone_mapping(params.hdr_buffer[offset], 0.6f);
    img[offset] = make_uchar4(fabsf(L.x) * 255, fabsf(L.y) * 255, fabsf(L.z) * 255, 0);
}

extern "C" void test(uchar4* img, cudaScene& scene, RenderParameters& params)
{
    dim3 blockSize(16, 16);
    dim3 gridSize(640 / blockSize.x, 480 / blockSize.y);

    if(params.iteration_count == 0)
    {
        checkCudaErrors(cudaMemset(params.hdr_buffer, 0, sizeof(float3) * WIDTH * HEIGHT));
    }

    testSimpleScene<<<gridSize, blockSize>>>(img, scene, params, wangHash(params.iteration_count));
    checkCudaErrors(cudaDeviceSynchronize());
}