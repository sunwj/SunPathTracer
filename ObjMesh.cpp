//
// Created by 孙万捷 on 16/4/24.
//

#include "ObjMesh.h"

ObjMesh::ObjMesh(std::string filename, Transformation& t)
{
    vmax = make_float3(-FLT_MAX);
    vmin = make_float3(FLT_MAX);

    Load(filename, t);
}

void ObjMesh::Load(std::string filename, Transformation& t)
{
    std::ifstream input(filename);
    if(!input)
    {
        std::cerr<<"Unable to open file: "<<filename<<std::endl;
        exit(-1);
    }

    char buffer[1024] = {0};
    while(input.getline(buffer, 1024))
    {
        if((buffer[0] == 'v') && (buffer[1] == ' '))
        {
            float3 v;
            sscanf(buffer, "v %f %f %f\n", &v.x, &v.y, &v.z);
            v = t * v;
            vmax = fmaxf(vmax, v);
            vmin = fminf(vmin, v);
            vertices.push_back(v);
        }
        else if((buffer[0] == 'f') && (buffer[1] == ' '))
        {
            uint3 f;
            sscanf(buffer, "f %u %u %u\n", &f.x, &f.y, &f.z);
            faces.push_back(f);
        }
        else
            continue;
    }

#define __PRINT_INFO__
#ifdef __PRINT_INFO__
    std::cout<<filename<<" load successfully"<<std::endl;
    std::cout<<"Number of vertices: "<<vertices.size()<<std::endl;
    std::cout<<"Number of faces: "<<faces.size()<<std::endl;
    std::cout<<"Mesh extent:"<<std::endl
        <<"max: ("<<vmax.x<<", "<<vmax.y<<", "<<vmax.z<<")"<<std::endl
        <<"min: ("<<vmin.x<<", "<<vmin.y<<", "<<vmin.z<<")"<<std::endl;
    std::cout<<"Mesh diagnal length: "<<length(vmax - vmin)<<std::endl;
#endif
}