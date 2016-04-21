//
// Created by 孙万捷 on 16/4/21.
//

#ifndef SUNPATHTRACER_KERNEL_GLOBALS_H
#define SUNPATHTRACER_KERNEL_GLOBALS_H

#include <cuda_runtime.h>

//marcos
#define IMG_BLACK make_uchar4(0, 0, 0, 0)

//data types
struct HitInfo
{
    float3 pt;
    float3 normal;
    unsigned int matID;
};

//functions
__inline__ __device__ void running_estimate(float3& acc_buffer, const float3& curr_est, unsigned int N)
{
    acc_buffer += (curr_est - acc_buffer) / (N + 1.f);
}

__host__ __device__ unsigned int wangHash(unsigned int a)
{
    //http://raytracey.blogspot.com/2015/12/gpu-path-tracing-tutorial-2-interactive.html
    a = (a ^ 61) ^ (a >> 16);
    a = a + (a << 3);
    a = a ^ (a >> 4);
    a = a * 0x27d4eb2d;
    a = a ^ (a >> 15);

    return a;
}

__device__ bool scene_intersect(const cudaScene& scene, const cudaRay& ray, HitInfo& hi)
{
    bool intersected = false;
    float tmin = FLT_MAX;

    for(int i = 0; i < scene.num_spheres; ++i)
    {
        float t;
        const cudaSphere& sphere = scene.spheres[i];
        if(sphere.Intersect(ray, &t) && (t < tmin))
        {
            tmin = t;
            intersected = true;

            hi.pt = ray.PointOnRay(tmin);
            hi.normal = sphere.GetNormal(hi.pt);
            hi.matID = sphere.material_id;
        }
    }

    for(int i = 0; i < scene.num_aabb_boxes; ++i)
    {
        float t;
        const cudaAABB& aabb = scene.aabb_boxes[i];
        if(aabb.Intersect(ray, &t) && (t < tmin))
        {
            tmin = t;
            intersected = true;

            hi.pt = ray.PointOnRay(tmin);
            hi.normal = aabb.GetNormal(hi.pt);
            hi.matID = aabb.material_id;
        }
    }

    for(int i = 0; i < scene.num_planes; ++i)
    {
        float t;
        const cudaPlane& plane = scene.planes[i];
        if(plane.Intersect(ray, &t) && (t < tmin))
        {
            tmin = t;
            intersected = true;

            hi.pt = ray.PointOnRay(tmin);
            hi.normal = plane.GetNormal(hi.pt);
            hi.matID = plane.material_id;
        }
    }

    return intersected;
}

#endif //SUNPATHTRACER_KERNEL_GLOBALS_H
