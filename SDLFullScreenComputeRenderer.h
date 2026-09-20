//
// Created by harish on 9/18/26.
//

#ifndef VOXEL_RAYTRACER_FULLSCREENRENDERER_H
#define VOXEL_RAYTRACER_FULLSCREENRENDERER_H


#include <iostream>
#include  <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>


class SDLFullScreenComputeRenderer {

private:
    SDL_Window* _window;
    SDL_GPUDevice* _device;
    SDL_GPUComputePipeline* _computePipeline;

    int _width =500,_height = 500;

public:
    SDLFullScreenComputeRenderer() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr<<"Failed to initialize SDL " << SDL_GetError() <<std::endl;
            return;
        }


        _window = SDL_CreateWindow("SDLWindow", _width,_height,SDL_WINDOW_RESIZABLE);

        if (!_window) {
            std::cerr<<"Failed to create window " << SDL_GetError() <<std::endl;
            return;
        }

        _device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV , true, NULL);

        if (!_device) {
            std::cerr<<"Failed to create gpu device " << SDL_GetError() <<std::endl;
            return;
        }



        SDL_ClaimWindowForGPUDevice( _device,_window);

        std::cout<<"Initialization successful" <<std::endl;

        _computePipeline = GetComputePipeline();


        Tick();
    }

    void Tick() {
        while (true) {
            SDL_Event event;
            if (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                    break;
                }
            }

            Triangle();
        }
        ExitApp();
    }


    void ExitApp() {
        SDL_DestroyGPUDevice(_device);
        SDL_DestroyWindow(_window);
        SDL_Quit();
    }

    void Triangle() {


        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(_device);

        SDL_GPUTexture* swapChainTexture;
        uint width = _width;
        uint height = _height;
        SDL_WaitAndAcquireGPUSwapchainTexture(cmd,_window,&swapChainTexture,&width,&height);

        if (!swapChainTexture) {

            SDL_SubmitGPUCommandBuffer(cmd);
            std::cout << "Swap chain texture is null" <<std::endl;
            return;
        }

        SDL_GPUTextureCreateInfo computeTextureInfo{};

        computeTextureInfo.height = 500;
        computeTextureInfo.layer_count_or_depth = 1;
        computeTextureInfo.width = 500;
        computeTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
        computeTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
        computeTextureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        computeTextureInfo.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        computeTextureInfo.num_levels = 1;

        SDL_GPUTexture *computeTexture = SDL_CreateGPUTexture(_device, &computeTextureInfo);

        if (!computeTexture) {
            std::cerr << "Output texture creation error " <<SDL_GetError()<<std::endl;
            return;
        }


        SDL_GPUStorageTextureReadWriteBinding computeTexBindings {};
        computeTexBindings.texture = computeTexture;
        computeTexBindings.mip_level = 0;
        computeTexBindings.layer = 0;
        computeTexBindings.cycle = true;

        SDL_GPUComputePass* computePass= SDL_BeginGPUComputePass( cmd, &computeTexBindings,1,nullptr,0);


        SDL_BindGPUComputePipeline( computePass, _computePipeline);
        SDL_DispatchGPUCompute( computePass,500,500,1);
        SDL_EndGPUComputePass( computePass);

        SDL_GPUColorTargetInfo colorTargetInfo{};

        colorTargetInfo.clear_color = { 1.0f,0.9f,0.8f,1.0f};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;

        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        colorTargetInfo.texture = swapChainTexture;

        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass( cmd,&colorTargetInfo,1, NULL);



        SDL_EndGPURenderPass(renderPass);

        SDL_GPUBlitInfo blitInfo{};
        blitInfo.cycle = true;
        blitInfo.source.texture = computeTexture;
        blitInfo.destination.texture = swapChainTexture;
        blitInfo.filter = SDL_GPU_FILTER_NEAREST;
        blitInfo.source.w = 500;
        blitInfo.source.h = 500;
        blitInfo.destination.w = 500;
        blitInfo.destination.h = 500;


        SDL_BlitGPUTexture(cmd,&blitInfo);

        SDL_SubmitGPUCommandBuffer(cmd);
    }

private :

    SDL_GPUComputePipeline* GetComputePipeline() {
        size_t spriVSize;

        uint8_t* computeShader = GetSpriVComputeShader("/home/harish/MyStuff/Projects/C++/voxel_raytracer/shaders/computeShader.hlsl",spriVSize);

        if (!computeShader) {
            std::cerr << "Compute shader creation failed" <<std::endl;
            return nullptr;
        }


        SDL_GPUComputePipelineCreateInfo computePipelineInfo{};

        computePipelineInfo.code = computeShader;
        computePipelineInfo.code_size = spriVSize;
        computePipelineInfo.entrypoint = "CSMain";
        computePipelineInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        computePipelineInfo.num_readwrite_storage_buffers = 0;
        computePipelineInfo.num_readwrite_storage_textures = 1;
        computePipelineInfo.threadcount_x = 8;
        computePipelineInfo.threadcount_y = 8;
        computePipelineInfo.threadcount_z = 1;


        auto res = SDL_CreateGPUComputePipeline(_device,&computePipelineInfo);

        if (!res) {
            std::cerr<< "Failed to create compute pipeline " << SDL_GetError()<<std::endl;
            return nullptr;
        }

        return res;
    }

    uint8_t* GetSpriVComputeShader(char* path,size_t &size) {

        size_t fileSize = 0;

        void* fileData = SDL_LoadFile(path, &fileSize);
        if (!fileData) {
            std::cerr << "Failed to load shader file at path: " << path
                      << " | SDL Error: " << SDL_GetError() << std::endl;
            return nullptr;
        }

        std::string shaderText(static_cast<char*>(fileData), fileSize);

        SDL_free(fileData);

        std::cout << "Loading shader file successful. Size: " << fileSize << " bytes" << std::endl;


        SDL_ShaderCross_HLSL_Info hlslInfo{};
        hlslInfo.entrypoint = "CSMain";
        hlslInfo.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;
        hlslInfo.source = shaderText.c_str();

        size_t spirVSize;
        void *gpuSpirV = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlslInfo, &spirVSize);

        if (!gpuSpirV) {
            std::cerr << "compilation to spirV failed " << SDL_GetError() << std::endl;
            return nullptr;
        }
        size = spirVSize;
        return static_cast<uint8_t *> (gpuSpirV);


        std::cout<<"Compilation successful"<<std::endl;

        SDL_GPUShaderCreateInfo shaderInfo{};
        shaderInfo.entrypoint = "main";
        shaderInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        shaderInfo.code = (uint8_t*)gpuSpirV;
        shaderInfo.code_size = spirVSize;

        SDL_GPUShader* computeShader = SDL_CreateGPUShader(_device, &shaderInfo);


        if (!computeShader) {
            std::cerr << "compilation to computeshader failed " << SDL_GetError() << std::endl;
            return nullptr;
        }
        SDL_free( gpuSpirV);

        std::cout << "Compilation to compute shader successful" <<std::endl;
        //return computeShader;
    }
};


#endif //VOXEL_RAYTRACER_FULLSCREENRENDERER_H