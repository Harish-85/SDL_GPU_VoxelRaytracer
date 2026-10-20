//
// Created by harish on 10/18/26.
//

#ifndef VOXEL_RAYTRACER_BUFFERHANDLER_H
#define VOXEL_RAYTRACER_BUFFERHANDLER_H
#include <vector>
#include <SDL3/SDL_gpu.h>

#include "ResourcePool.h"
#include "SDLUploadContext.h"

struct GPUBufferTag{};

using GPUBufferHandle = Handle<GPUBufferTag>;

class SDLBufferHandler {
private:
    SDL_GPUDevice* _device;
    SDLUploadContext* _uploadContext;
    ResourcePool<SDL_GPUBuffer*,GPUBufferTag> _buffers;
public:
    explicit SDLBufferHandler(SDL_GPUDevice* device,SDLUploadContext* uploadContext);

    SDL_GPUBuffer* GetBuffer( const GPUBufferHandle handle) {return _buffers.Get(handle);}

    uint32_t AllocateRawBuffer(size_t size, SDL_GPUBufferUsageFlags usage);

    GPUBufferHandle CreateVertexBuffer(size_t size);

    GPUBufferHandle CreateIndexBuffer(size_t size);

    GPUBufferHandle CreateComputeBuffer(size_t size, bool allowWrite = false);

    void UploadBufferData(const GPUBufferHandle handle, const void* data, size_t size) const;

    void DestroyBuffer(GPUBufferHandle handle);
};


#endif //VOXEL_RAYTRACER_BUFFERHANDLER_H