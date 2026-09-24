//
// Created by harish on 9/18/26.
//

#ifndef VOXEL_RAYTRACER_FULLSCREENRENDERER_H
#define VOXEL_RAYTRACER_FULLSCREENRENDERER_H


#include <iostream>
#include  <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "VoxelData.h"

struct CameraData {
    glm::vec3 position;
    glm::vec3 front;
    float yaw = -90;
    float pitch;
};

struct PushConstants {
    glm::vec4 origin;
    glm::vec4 direction;
    uint32_t height,width,depth;
    uint32_t padding;
};

class SDLFullScreenComputeRenderer {

private:
    SDL_Window* _window;
    SDL_GPUDevice* _device;
    SDL_GPUComputePipeline* _computePipeline;
    SDL_GPUTexture* _computeTexture;
    SDL_GPUBuffer* voxelBuffer;
    SDL_GPUBuffer* colorBuffer;
    SDL_GPUTransferBuffer* colorStagingBuff;
    SDL_GPUTransferBuffer* stagingBuffer;
    std::vector<uint32_t> voxels;
    Color colors[256];

    int _gridHeight,_girdDepth,_gridWidth;

    CameraData cam { glm::vec3(50,50,50),glm::vec3(0,1,0),-90.0f,0.0f};

    int _width =1000,_height = 1000;

public:
    SDLFullScreenComputeRenderer(std::vector<uint32_t> v,Color c[256],int width,int height,int depth) {
        voxels = v;
        std::copy(c, c + 256, std::begin(colors));
        _girdDepth = depth;
        _gridHeight = height;
        _gridWidth = width;

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
        _computeTexture = SetupComputeTexture();

        UploadGpuBuffToStagingBuff(voxelBuffer,stagingBuffer,voxels.size() * sizeof(uint32_t),voxels.data());
        UploadGpuBuffToStagingBuff(colorBuffer,colorStagingBuff,256 * sizeof(uint8_t) *4,colors);

        Tick();
    }

    bool isRightClickHeld;
    float sensitivity = 1;
    bool isRunning = true;
    float moveSpeed = .1;
    void Tick() {
        while (isRunning) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                    isRunning = false;
                    break;
                }

                if ( event.type == SDL_EVENT_MOUSE_MOTION && event.motion.state & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
                    /*glm::vec2 mouseDelta = glm::vec2(event.motion.xrel,event.motion.yrel);

                    glm::vec3 right = glm::normalize(glm::vec3(cam.transform[0]));
                    auto res = glm::rotate( cam.transform,sensitivity*mouseDelta.x,glm::vec3(0,1,0));

                    res = glm::rotate( res,sensitivity*mouseDelta.y,right);


                    cam.transform = res;

                    glm::quat rotationQuat = glm::quat_cast(res);
                    glm::vec3 eulerAngles = glm::eulerAngles(rotationQuat);

                    float pitchDegrees = glm::degrees(eulerAngles.x);
                    float yawDegrees   = glm::degrees(eulerAngles.y);
                    float rollDegrees  = glm::degrees(eulerAngles.z);

                    std::cout << "Camera rotation " << pitchDegrees << " " << yawDegrees << " " << rollDegrees <<std::endl;
*/
                    cam.yaw   -= event.motion.xrel * sensitivity;
                    cam.pitch -= event.motion.yrel * sensitivity;

                    if (cam.pitch > 89.0f)  cam.pitch = 89.0f;
                    if (cam.pitch < -89.0f) cam.pitch = -89.0f;

                    glm::vec3 front;
                    front.x = cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
                    front.y = sin(glm::radians(cam.pitch));
                    front.z = sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
                    front = glm::normalize(front);

                    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
                    glm::vec3 up    = glm::normalize(glm::cross(right, front));

                    cam.front = front;

                }


            }
            const bool* keyboardState = SDL_GetKeyboardState(NULL);

            // Calculate dynamic directional vectors relative to camera looking angle
            glm::vec3 forwardDir = cam.front;
            glm::vec3 rightDir   = glm::normalize(glm::cross(forwardDir, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 upDir      = glm::vec3(0.0f, 1.0f, 0.0f); // Pure world-up for E/Q vertical movement

            float currentSpeed = moveSpeed ;

            if (keyboardState[SDL_SCANCODE_W]) {
                cam.position += forwardDir * currentSpeed;
            }
            if (keyboardState[SDL_SCANCODE_S]) {
                cam.position -= forwardDir * currentSpeed;
            }
            if (keyboardState[SDL_SCANCODE_A]) {
                cam.position += rightDir * currentSpeed;
            }
            if (keyboardState[SDL_SCANCODE_D]) {
                cam.position -= rightDir * currentSpeed;
            }
            // Optional: E to go Up, Q to go Down
            if (keyboardState[SDL_SCANCODE_E]) {
                cam.position += upDir * currentSpeed;
            }
            if (keyboardState[SDL_SCANCODE_Q]) {
                cam.position -= upDir * currentSpeed;
            }

            Triangle();
        }
        ExitApp();
    }


    void ExitApp() {
        SDL_ReleaseGPUTexture( _device,_computeTexture);
        SDL_ReleaseGPUComputePipeline(_device,_computePipeline);

        SDL_DestroyGPUDevice(_device);
        SDL_DestroyWindow(_window);
        SDL_Quit();
    }


    void CopyStagingBuffToGpuSize(SDL_GPUCommandBuffer *cmd,SDL_GPUBuffer* gpuBuff,SDL_GPUTransferBuffer* transferBuffer,uint size) {
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass( cmd);
        SDL_GPUTransferBufferLocation transferLocation{};
        transferLocation.transfer_buffer = transferBuffer;
        transferLocation.offset = 0;

        SDL_GPUBufferRegion destination{};
        destination.buffer = gpuBuff;
        destination.size = size;
        destination.offset = 0;

        SDL_UploadToGPUBuffer( copyPass, &transferLocation,&destination,false);
        SDL_EndGPUCopyPass(copyPass);
    }

    void Triangle() {


        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(_device);

        //copy the voxel data
        CopyStagingBuffToGpuSize(cmd,voxelBuffer,stagingBuffer,voxels.size() * sizeof(uint32_t));
        CopyStagingBuffToGpuSize(cmd,colorBuffer,colorStagingBuff,256 * sizeof(Color));


        SDL_GPUTexture* swapChainTexture;
        uint width = _width;
        uint height = _height;
        SDL_WaitAndAcquireGPUSwapchainTexture(cmd,_window,&swapChainTexture,&width,&height);

        if (!swapChainTexture) {

            SDL_SubmitGPUCommandBuffer(cmd);
            std::cout << "Swap chain texture is null" <<std::endl;
            return;
        }


        SDL_GPUStorageTextureReadWriteBinding computeTexBindings {};
        computeTexBindings.texture = _computeTexture;
        computeTexBindings.mip_level = 0;
        computeTexBindings.layer = 0;
        computeTexBindings.cycle = true;


        computeTexBindings.texture = _computeTexture;
        computeTexBindings.mip_level = 0;
        computeTexBindings.layer = 0;
        computeTexBindings.cycle = true;


        SDL_GPUComputePass* computePass= SDL_BeginGPUComputePass( cmd, &computeTexBindings,1,
            nullptr,0);

        SDL_BindGPUComputePipeline( computePass, _computePipeline);
        PushConstants constants{};
        constants.direction = glm::vec4(cam.front, 0.0f);
        //constants.origin    = glm::vec4(glm::vec3(cam.transform[3]), 0.0f);
        constants.origin    = glm::vec4(cam.position,0.0f);

        constants.height = _gridHeight;
        constants.depth = _girdDepth;
        constants.width = _gridWidth;

        std::cout<< "Pushing values to compute shader "<<std::endl;
        std::cout
    << "width  = " << constants.width << '\n'
    << "height = " << constants.height << '\n'
    << "depth  = " << constants.depth << '\n'
    << "sizeof = " << sizeof(PushConstants) << '\n';
        SDL_PushGPUComputeUniformData( cmd, 0,&constants,sizeof(constants));

        SDL_BindGPUComputeStorageBuffers( computePass,0,&voxelBuffer,1);
        SDL_BindGPUComputeStorageBuffers( computePass,1,&colorBuffer,1);

        SDL_DispatchGPUCompute( computePass,width/8,height/8,1);
        SDL_EndGPUComputePass( computePass);
        std::cout<< "Compute shader done "<<SDL_GetError() <<std::endl;



        SDL_GPUColorTargetInfo colorTargetInfo{};

        colorTargetInfo.clear_color = { 1.0f,0.9f,0.8f,1.0f};
        colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;

        colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
        colorTargetInfo.texture = swapChainTexture;

       // SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass( cmd,&colorTargetInfo,1, NULL);



        //SDL_EndGPURenderPass(renderPass);

        SDL_GPUBlitInfo blitInfo{};
        blitInfo.cycle = true;
        blitInfo.source.texture = _computeTexture;
        blitInfo.destination.texture = swapChainTexture;
        blitInfo.filter = SDL_GPU_FILTER_NEAREST;
        blitInfo.source.w = width;
        blitInfo.source.h = height;
        blitInfo.destination.w = width;
        blitInfo.destination.h = height;


        SDL_BlitGPUTexture(cmd,&blitInfo);

        SDL_SubmitGPUCommandBuffer(cmd);


    }

private :
    SDL_GPUTexture *SetupComputeTexture() {
        SDL_GPUTextureCreateInfo computeTextureInfo{};

        computeTextureInfo.height = _height;
        computeTextureInfo.layer_count_or_depth = 1;
        computeTextureInfo.width = _width;
        computeTextureInfo.type = SDL_GPU_TEXTURETYPE_2D;
        computeTextureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
        computeTextureInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
        computeTextureInfo.usage = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        computeTextureInfo.num_levels = 1;

        SDL_GPUTexture *computeTexture = SDL_CreateGPUTexture(_device, &computeTextureInfo);

        if (!computeTexture) {
            std::cerr << "Output texture creation error " <<SDL_GetError()<<std::endl;
            return nullptr;
        }

        return computeTexture;
    }

    void UploadGpuBuffToStagingBuff(SDL_GPUBuffer* &buff, SDL_GPUTransferBuffer* &tranferBuff,uint size,void* data) {

        SDL_GPUBufferCreateInfo voxelBuffInfo{};
        voxelBuffInfo.props = 0;
        voxelBuffInfo.size = size;
        voxelBuffInfo.usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ;


        buff = SDL_CreateGPUBuffer(_device,&voxelBuffInfo);

        SDL_GPUStorageBufferReadWriteBinding computeBuffBindings{};
        computeBuffBindings.buffer = buff;

        SDL_GPUTransferBufferCreateInfo transferInfo {};
        transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
        transferInfo.size = size;
        tranferBuff = SDL_CreateGPUTransferBuffer(_device,&transferInfo);

        void* dataPtr = SDL_MapGPUTransferBuffer(_device,tranferBuff,false);
        SDL_memcpy( dataPtr,data,size);
        SDL_UnmapGPUTransferBuffer(_device, tranferBuff);


    }

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
        computePipelineInfo.num_uniform_buffers = 1;
        computePipelineInfo.num_readonly_storage_buffers = 2;
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