//
// Created by harish on 9/27/26.
//

#ifndef VOXEL_RAYTRACER_GAME_H
#define VOXEL_RAYTRACER_GAME_H
#include <memory>
#include <vector>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>

#include "Camera.h"
#include "Object3D.h"
#include "SDLFullScreenComputeRenderer.h"


class Game {
private:
    std::vector<std::unique_ptr<Object3D>> objects;
    FreeCamera* camera;

    bool isRunning = true;

    std::vector<uint32_t> voxels;
    Color color[256];

    //subsystems
    std::unique_ptr<SDLFullScreenComputeRenderer> _renderer;

public:
    Game(VoxelData data,Color colors[256]) {
        this->voxels = data.Voxels;
        memcpy(color,colors,256*sizeof(Color));
        _renderer = std::make_unique<SDLFullScreenComputeRenderer>(data.BoundsWidth,data.BoundsHeight,data.BoundsDepth );

        std::unique_ptr<FreeCamera> cam = std::make_unique<FreeCamera>();
        camera = cam.get();
        InstantiateObject(std::move(cam) );
    }


    Uint64 lastTime = SDL_GetTicksNS();
    float deltaTime;
    void MainLoop() {


        _renderer->UploadColorToGpu(color);
        _renderer->UploadVoxelsToGpu(voxels);
        while (isRunning) {


            if (SDL_HasEvent(SDL_EVENT_WINDOW_CLOSE_REQUESTED) || SDL_HasEvent(SDL_EVENT_QUIT)) {
                isRunning = false;
            }
            const Uint64 currentTime = SDL_GetTicksNS();
            const Uint64 elapsedTime = currentTime - lastTime;
            lastTime = currentTime;
            deltaTime = (float) elapsedTime / 1000000000.f;

            for (auto  &object : objects) {
                object->Tick(deltaTime);
            }
            _renderer->SetCameraDirection(camera->GetForward());
            _renderer->SetCameraPosition(camera->GetPosition());




            _renderer->Render();
        }
    }

    void InstantiateObject(std::unique_ptr<Object3D> object) {

        Object3D* ptr = object.get();
        objects.push_back( std::move( object));

        ptr->Start();
    }
};

#endif //VOXEL_RAYTRACER_GAME_H