//
// Created by harish on 10/19/26.
//

#ifndef VOXEL_RAYTRACER_SDLTEXTUREHANDLER_H
#define VOXEL_RAYTRACER_SDLTEXTUREHANDLER_H
#include <vector>
#include <SDL3/SDL_gpu.h>

#include "ResourcePool.h"
#include "SDLUploadContext.h"

struct GPUTexture;

using GPUTextureHandle = Handle<GPUTexture>;

class SDLTextureHandler {
private:
    SDL_GPUDevice* _device;
    SDLUploadContext* _uploadContext;

    ResourcePool<SDL_GPUTexture*,GPUTexture> _texturePool;

public:
    explicit SDLTextureHandler(SDL_GPUDevice* device,SDLUploadContext* uploadContext):
    _device(device),_uploadContext(uploadContext) {}


    GPUTextureHandle CreateTexture(const SDL_GPUTextureCreateInfo &textureCreateInfo);

    void UploadTextureData(GPUTextureHandle handle, const void* data, uint32_t width, uint32_t height,uint32_t bytesPerPixel) const;

    void DestroyTexture(GPUTextureHandle handle);

};


#endif //VOXEL_RAYTRACER_SDLTEXTUREHANDLER_H