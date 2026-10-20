//
// Created by harish on 10/18/26.
//

#ifndef VOXEL_RAYTRACER_SDL_RENDERER_H
#define VOXEL_RAYTRACER_SDL_RENDERER_H
#include <memory>
#include <stdexcept>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include "GameWindow.h"
#include "IRenderer.h"
#include "SDLBufferHandler.h"
#include "SDLTextureHandler.h"
#include "SDLUploadContext.h"


class SDLRenderer : IRenderer{

private:
    SDL_GPUDevice* _device;
    std::unique_ptr<SDLUploadContext> _uploadContext;
    std::unique_ptr<SDLBufferHandler> _bufferHandler;
    std::unique_ptr<SDLTextureHandler> _textureHandler;

public:

    SDLRenderer(const GameWindow* targetWindow) {

        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetBooleanProperty(props,SDL_PROP_GPU_DEVICE_CREATE_PREFERLOWPOWER_BOOLEAN,false);
        SDL_SetBooleanProperty(props,SDL_PROP_GPU_DEVICE_CREATE_SHADERS_SPIRV_BOOLEAN,true);

        //_device = SDL_CreateGPUDeviceWithProperties(SDL_GPU_SHADERFORMAT_SPIRV , true, NULL);
        _device = SDL_CreateGPUDeviceWithProperties(props);

        if (!_device) {
            throw std::runtime_error( std::string("Failed to create gpu device :") + SDL_GetError());
            return;
        }

        SDL_ClaimWindowForGPUDevice( _device,targetWindow->GetWindow());

        _uploadContext = std::make_unique<SDLUploadContext>(_device);
        _bufferHandler = std::make_unique<SDLBufferHandler>(_device,_uploadContext.get());
        _textureHandler = std::make_unique<SDLTextureHandler>(_device,_uploadContext.get());

    }

    [[nodiscard]] GPUBufferHandle CreateComputeBuffer( const size_t size,const bool allowWrite = false) const {
        return _bufferHandler->CreateComputeBuffer(size,allowWrite);
    }
    [[nodiscard]] GPUBufferHandle CreateVertexBuffer(const size_t size) const {
        return _bufferHandler->CreateVertexBuffer(size);
    }
    [[nodiscard]] GPUBufferHandle CreateIndexBuffer(const size_t size) const {
        return _bufferHandler->CreateIndexBuffer(size);
    }

    SDL_GPUBuffer* GetComputeBuffer(GPUBufferHandle buff) {
        return _bufferHandler->GetBuffer(buff);
    }

    void UploadDataToBuffer(const GPUBufferHandle handle,const void* data ,const size_t size) const {
        return _bufferHandler->UploadBufferData( handle, data,size);
    }

    SDL_GPUDevice * GetDevice() { return _device;}
};


#endif //VOXEL_RAYTRACER_SDL_RENDERER_H