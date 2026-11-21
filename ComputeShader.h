//
// Created by harish on 11/19/26.
//
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <SDL3_shadercross/SDL_shadercross.h>

class ComputeShader {
public:
    ComputeShader(std::string shaderPath, SDL_GPUDevice* device) {
        size_t spirVSize;
        uint8_t* computeShader = GetSpirVFromShader( shaderPath,spirVSize);

        SDL_ShaderCross_ComputePipelineMetadata *res = SDL_ShaderCross_ReflectComputeSPIRV(computeShader, spirVSize, 0);

        SDL_GPUComputePipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.entrypoint = "CSMain";
        pipelineCreateInfo.code = computeShader;
        pipelineCreateInfo.code_size = spirVSize;
        pipelineCreateInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;

        pipelineCreateInfo.num_readonly_storage_buffers = res->num_readonly_storage_buffers;
        pipelineCreateInfo.num_readonly_storage_textures = res->num_readonly_storage_textures;
        pipelineCreateInfo.num_readwrite_storage_buffers = res->num_readwrite_storage_buffers;
        pipelineCreateInfo.num_readwrite_storage_textures = res->num_readwrite_storage_textures;
        pipelineCreateInfo.num_samplers = res->num_samplers;
        pipelineCreateInfo.num_uniform_buffers = res->num_uniform_buffers;
        pipelineCreateInfo.threadcount_x = res->threadcount_x;
        pipelineCreateInfo.threadcount_y = res->threadcount_y;
        pipelineCreateInfo.threadcount_z = res->threadcount_z;

        std::cout << std::format(
                    "Compute shader reflection res : ro buffs {}, ro textures {}, rw buffs {}, rw textures {}, samplers {}, uniform buffs {}, threadcount ({}, {}, {})\n",
                    res->num_readonly_storage_buffers,
                    res->num_readonly_storage_textures,
                    res->num_readwrite_storage_buffers,
                    res->num_readwrite_storage_textures,
                    res->num_samplers,
                    res->num_uniform_buffers,
                    res->threadcount_x,
                    res->threadcount_y,
                    res->threadcount_z
                );

        SDL_free( res);

        _computePipeline = SDL_CreateGPUComputePipeline(device, &pipelineCreateInfo);

        if (!_computePipeline) {
            std::cerr << "Compute pipeline creation failed " <<SDL_GetError() <<std::endl;
            throw std::runtime_error( SDL_GetError());
        }
    }

    void SetReadWriteBuffer(uint32_t slot, SDL_GPUBuffer* buff) {
        SDL_GPUStorageBufferReadWriteBinding readWriteTexBinding{};
        readWriteTexBinding.buffer = buff;
        readWriteTexBinding.cycle = false;


        if ( slot >= readWriteBuffs.size())
            readWriteBuffs.resize( slot + 1 );
        readWriteBuffs[slot] = readWriteTexBinding;
    }

    void SetReadOnlyBuffer(uint32_t slot, SDL_GPUBuffer* buff) {
        if ( slot >= readOnlyBuffs.size())
            readOnlyBuffs.resize( slot + 1 , nullptr);
        readOnlyBuffs[slot] = buff;
    }

    void SetReadOnlyTexture(uint32_t slot, SDL_GPUTexture* tex) {
        if ( slot >= readOnlyTex.size()) {
            readOnlyTex.resize( slot+1,nullptr);
        }
        readOnlyTex[slot] = tex;
    }

    void SetReadWriteTexture(uint32_t slot, SDL_GPUTexture* tex) {
        SDL_GPUStorageTextureReadWriteBinding readWriteTexBinding{};
        readWriteTexBinding.texture = tex;
        readWriteTexBinding.cycle = false;

        if ( slot >= readWriteTex.size()) {
            readWriteTex.resize( slot+1);
        }
        readWriteTex[slot] = readWriteTexBinding;
    }

    void SetPushConstants(void* constant, size_t size) {
        pushConstant = constant;
        pushConstSize = size;
    }

    void Dispatch(SDL_GPUCommandBuffer* cmd, uint32_t x, uint32_t y ,uint32_t z) {
        std::cout << "Dis[atching compute shader" <<std::endl;
        SDL_GPUComputePass *computePass = SDL_BeginGPUComputePass(cmd, readWriteTex.data(), readWriteTex.size(),
                                                                  readWriteBuffs.data(), readWriteBuffs.size());

        SDL_BindGPUComputePipeline( computePass, _computePipeline);

        if (pushConstant)
            SDL_PushGPUComputeUniformData( cmd, 0,pushConstant,pushConstSize);

        std::cout << "set push constants" <<std::endl;
        // read only buff and tex
        for (int i = 0 ; i< readOnlyTex.size(); i++) {
            if (readOnlyTex[i] == nullptr) {
                continue;
            }

            SDL_BindGPUComputeStorageTextures( computePass,i,&readOnlyTex[i],1);
        }
        std::cout << "set readonly buff tex" <<std::endl;
        for (int i = 0 ; i< readOnlyBuffs.size(); i++) {
            if (readOnlyBuffs[i] == nullptr) {
                continue;
            }

            SDL_BindGPUComputeStorageBuffers( computePass,i,&readOnlyBuffs[i],1);
        }
        std::cout << "set readonly buff " <<std::endl;

        SDL_DispatchGPUCompute( computePass,x,y,z);
        SDL_EndGPUComputePass(computePass);


    }

private:
    SDL_GPUComputePipeline* _computePipeline;

    std::vector<SDL_GPUBuffer*> readOnlyBuffs;
    std::vector<SDL_GPUStorageBufferReadWriteBinding> readWriteBuffs;


    std::vector<SDL_GPUTexture*> readOnlyTex;
    std::vector<SDL_GPUStorageTextureReadWriteBinding> readWriteTex;

    void* pushConstant;
    size_t pushConstSize;

    uint8_t* GetSpirVFromShader(std::string& path, size_t &size) {
        size_t fileSize = 0;

        void* fileData = SDL_LoadFile(path.c_str(),&fileSize);

        if (!fileData) {
            std::cerr<<"Failed to load shader file at path "<<path
            << " | SDL Error: " << SDL_GetError() << std::endl;
            return nullptr;
        }
        std::string shaderText( static_cast<char*>(fileData),fileSize);

        SDL_free(fileData);

        SDL_ShaderCross_HLSL_Info hlslInfo{};
        hlslInfo.source = shaderText.c_str();
        hlslInfo.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;
        hlslInfo.entrypoint = "CSMain";


        size_t spirVSize;
        void* gpuSpirV =  SDL_ShaderCross_CompileSPIRVFromHLSL( &hlslInfo,&spirVSize);

        if (!gpuSpirV) {
            std::cerr << "compilation to spirV failed " << SDL_GetError() << std::endl;
            return nullptr;
        }

        size = spirVSize;

        return static_cast<uint8_t *> ( gpuSpirV);

    }


};
