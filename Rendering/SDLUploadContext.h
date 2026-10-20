//
// Created by harish on 10/20/26.
//

#ifndef VOXEL_RAYTRACER_UPLOADCONTEXT_H
#define VOXEL_RAYTRACER_UPLOADCONTEXT_H
#include <cstring>
#include <SDL3/SDL_gpu.h>


class SDLUploadContext {
public:
    SDLUploadContext(SDL_GPUDevice* device) : _device(device) {  }

    template <typename RecordFn>
    void Upload(const void* data, size_t size, RecordFn record) {
        SDL_GPUTransferBufferCreateInfo transferBuffInfo{};

        transferBuffInfo.size = static_cast<uint32_t>(size);
        transferBuffInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;

        SDL_GPUTransferBuffer* tb = SDL_CreateGPUTransferBuffer(_device,&transferBuffInfo);
        if (!tb) {
            return;
        }

        void* mapped = SDL_MapGPUTransferBuffer( _device,tb,false);
        std::memcpy( mapped, data, size);
        SDL_UnmapGPUTransferBuffer(_device,tb);

        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer( _device);
        SDL_GPUCopyPass* pass = SDL_BeginGPUCopyPass(cmd);
        record(pass,tb);
        SDL_EndGPUCopyPass(pass);
        SDL_SubmitGPUCommandBuffer(cmd);

        SDL_ReleaseGPUTransferBuffer(_device,tb);

    }

private:
    SDL_GPUDevice* _device;
};


#endif //VOXEL_RAYTRACER_UPLOADCONTEXT_H