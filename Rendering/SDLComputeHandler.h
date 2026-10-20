//
// Created by harish on 10/19/26.
//

#ifndef VOXEL_RAYTRACER_SDLCOMPUTEHANDLER_H
#define VOXEL_RAYTRACER_SDLCOMPUTEHANDLER_H
#include <cstdint>
#include <string>
#include <vector>
#include <SDL3/SDL_gpu.h>

#include "ResourcePool.h"

struct ComputePipelineDesc {
    void* spirVcode;
    std::size_t spirVSize;
    std::string entryPoint;
    uint32_t readOnlyBuffers  = 0;
    uint32_t readWriteBuffers = 0;
    uint32_t readOnlyTextures = 0;
    uint32_t readWriteTextures = 0;
    uint32_t uniformBuffers   = 0;
    uint32_t threadsX = 1, threadsY = 1, threadsZ = 1;
};

struct ComputeTag;
using ComputeHandle = Handle<ComputeTag>;


class SDLComputeHandler {
public:
    SDLComputeHandler(SDL_GPUDevice* dev) : _device(dev) {  }

    ComputeHandle CreatePipeline(const ComputePipelineDesc &desc)   {
        SDL_GPUComputePipelineCreateInfo computePipelineInfo{};

        computePipelineInfo.code = static_cast<uint8_t*> (desc.spirVcode);
        computePipelineInfo.code_size = desc.spirVSize;
        computePipelineInfo.entrypoint = desc.entryPoint.c_str();
        computePipelineInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;

        computePipelineInfo.num_readonly_storage_textures = desc.readOnlyTextures;
        computePipelineInfo.num_readwrite_storage_textures = desc.readWriteTextures;

        computePipelineInfo.num_readonly_storage_buffers = desc.readOnlyBuffers;
        computePipelineInfo.num_readwrite_storage_buffers = desc.readWriteBuffers;

        computePipelineInfo.num_uniform_buffers = desc.uniformBuffers;
        computePipelineInfo.threadcount_x = desc.threadsX;
        computePipelineInfo.threadcount_y = desc.threadsY;
        computePipelineInfo.threadcount_z = desc.threadsZ;

        SDL_GPUComputePipeline* pipeline = SDL_CreateGPUComputePipeline(_device,&computePipelineInfo);

        if (!pipeline) {
            return {};
        }

        _activePipelines.push_back(pipeline);
        const uint32_t index = _activePipelines.size();

        return ComputeHandle {index};
    }


private:
    SDL_GPUDevice* _device;

    std::vector<SDL_GPUComputePipeline*> _activePipelines;

};


#endif //VOXEL_RAYTRACER_SDLCOMPUTEHANDLER_H