//
// Created by harish on 10/18/26.
//

#include "SDLBufferHandler.h"

#include <cstring>

SDLBufferHandler::SDLBufferHandler(SDL_GPUDevice *device,SDLUploadContext* uploadContext):_device(device), _uploadContext(uploadContext){   }

uint32_t SDLBufferHandler::AllocateRawBuffer(size_t size, SDL_GPUBufferUsageFlags usage) {
    SDL_GPUBufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    bufferInfo.usage = usage;

    SDL_GPUBuffer* buff = SDL_CreateGPUBuffer( _device,&bufferInfo);



    return    (_buffers.Add(buff).id);
}

GPUBufferHandle SDLBufferHandler::CreateVertexBuffer(const size_t size) {
    uint32_t id = AllocateRawBuffer( size, SDL_GPU_BUFFERUSAGE_VERTEX);
    return GPUBufferHandle{id};
}

GPUBufferHandle SDLBufferHandler::CreateIndexBuffer(const size_t size) {
    uint32_t id = AllocateRawBuffer( size, SDL_GPU_BUFFERUSAGE_INDEX);
    return GPUBufferHandle{id};
}

GPUBufferHandle SDLBufferHandler::CreateComputeBuffer(const size_t size, bool allowWrite) {
    SDL_GPUBufferUsageFlags flags = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ;
    if (allowWrite) {
        flags |= SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE;
    }

    uint32_t id = AllocateRawBuffer(size, flags);
    return GPUBufferHandle{id};
}


void SDLBufferHandler::UploadBufferData(const GPUBufferHandle handle, const void *data, const size_t size) const {
    SDL_GPUBuffer* targetBuffer = _buffers.Get(handle);

    _uploadContext->Upload( data,size,
        [&](SDL_GPUCopyPass* pass, SDL_GPUTransferBuffer* tb){
        SDL_GPUTransferBufferLocation transferBuffLocation{};
        transferBuffLocation.offset =0;
        transferBuffLocation.transfer_buffer = tb;

        SDL_GPUBufferRegion destRegion{};
        destRegion.offset =0;
        destRegion.buffer = targetBuffer;
        destRegion.size = size;

            SDL_UploadToGPUBuffer( pass,&transferBuffLocation, &destRegion,true);
    });

}

void SDLBufferHandler::DestroyBuffer(GPUBufferHandle handle) {


    if (_buffers.Get(handle)) {
        _buffers.Remove(handle);
    }

}
