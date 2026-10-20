//
// Created by harish on 10/18/26.
//

#ifndef VOXEL_RAYTRACER_BUFFERHANDLER_H
#define VOXEL_RAYTRACER_BUFFERHANDLER_H
#include <vector>
#include <SDL3/SDL_gpu.h>

#include "SDLUploadContext.h"


class GPUBufferHandle {

public:
    uint32_t id = 0;
    [[nodiscard]] bool IsValid() const noexcept { return id != 0; }
};


class SDLBufferHandler {
private:
    SDL_GPUDevice* _device;
    SDLUploadContext* _uploadContext;
    std::vector<SDL_GPUBuffer*> _buffers;
    uint32_t _nextBufferId = 1;
public:
    explicit SDLBufferHandler(SDL_GPUDevice* device,SDLUploadContext* uploadContext);


    uint32_t AllocateRawBuffer(size_t size, SDL_GPUBufferUsageFlags usage);

    GPUBufferHandle CreateVertexBuffer(size_t size);

    GPUBufferHandle CreateIndexBuffer(size_t size);

    GPUBufferHandle CreateComputeBuffer(size_t size, bool allowWrite = false);

    void UploadBufferData(const GPUBufferHandle handle, const void* data, size_t size) const;

    void DestroyBuffer(GPUBufferHandle handle);
};


#endif //VOXEL_RAYTRACER_BUFFERHANDLER_H