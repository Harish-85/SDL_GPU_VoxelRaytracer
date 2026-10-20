//
// Created by harish on 10/19/26.
//

#include "SDLTextureHandler.h"

#include <cstring>
#include <SDL3/SDL_render.h>


GPUTextureHandle SDLTextureHandler::CreateTexture(const SDL_GPUTextureCreateInfo &textureCreateInfo) {
    SDL_GPUTexture *texture = SDL_CreateGPUTexture(_device, &textureCreateInfo);

    if (!texture) {
        return {};
    }

    return _texturePool.Add(texture);
}

void SDLTextureHandler::UploadTextureData(GPUTextureHandle handle, const void *data, uint32_t width, uint32_t height,
                                          uint32_t bytesPerPixel) const {
    SDL_GPUTexture *target = _texturePool.Get(handle);

    _uploadContext->Upload(data, width * height * bytesPerPixel,
                           [&](SDL_GPUCopyPass *copyPass, SDL_GPUTransferBuffer *tb) {
                               SDL_GPUTextureTransferInfo src{};
                               src.transfer_buffer = tb;
                               src.offset = 0;
                               src.pixels_per_row = width;
                               src.rows_per_layer = height;

                               SDL_GPUTextureRegion dst{};
                               dst.texture = target;
                               dst.mip_level = 0;
                               dst.layer = 0;
                               dst.x = 0;
                               dst.y = 0;
                               dst.z = 0;
                               dst.w = width;
                               dst.h = height;
                               dst.d = 1;

                               SDL_UploadToGPUTexture(copyPass, &src, &dst, false);
                           });
}

void SDLTextureHandler::DestroyTexture(const GPUTextureHandle handle) {
    if (SDL_GPUTexture *tex = _texturePool.Get(handle)) {
        SDL_ReleaseGPUTexture(_device, tex);
        _texturePool.Remove(handle);
    }
}
