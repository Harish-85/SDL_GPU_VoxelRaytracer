//
// Created by harish on 10/18/26.
//

#ifndef VOXEL_RAYTRACER_IRENDERER_H
#define VOXEL_RAYTRACER_IRENDERER_H

enum BufferBindFlags : uint32_t {
    Bind_None         = 0,
    Bind_Vertex       = 1 << 0,
    Bind_Index        = 1 << 1,
    Bind_ComputeRead  = 1 << 2,
    Bind_ComputeWrite = 1 << 3
};

class IRenderer {
public:
    void Init();

};


#endif //VOXEL_RAYTRACER_IRENDERER_H