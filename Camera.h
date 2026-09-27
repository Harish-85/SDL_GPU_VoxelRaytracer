//
// Created by harish on 9/27/26.
//

#ifndef VOXEL_RAYTRACER_CAMERA_H
#define VOXEL_RAYTRACER_CAMERA_H

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>

#include "ITickable.h"
#include "Object3D.h"

class FreeCamera : public Object3D{

private:
    float moveSpeed = 12.0f;
    float _sensitivity = 2.0f;
    float yaw=0,pitch = 0;
public:
    FreeCamera() {

    }
    void Start() override {

    }

    void Tick(float deltaTime) override {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            //set rotation
            if ( event.type == SDL_EVENT_MOUSE_MOTION && event.motion.state & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
                yaw   -= event.motion.xrel * _sensitivity;
                pitch -= event.motion.yrel * _sensitivity;

                if (pitch > 89.0f)  pitch = 89.0f;
                if (pitch < -89.0f) pitch = -89.0f;

                //make the quaternions
                glm::quat qPitch = glm::angleAxis( glm::radians(pitch), glm::vec3(1.0,0.0,0.0));
                glm::quat qYaw = glm::angleAxis( glm::radians (yaw), glm::vec3(0.0,1.0,0.0));

                SetRotation( qYaw * qPitch);
            }
        }

        //position
            const bool* keyboardState = SDL_GetKeyboardState(NULL);

            // Calculate dynamic directional vectors relative to camera looking angle
            glm::vec3 forwardDir = GetForward();
            glm::vec3 rightDir   = glm::normalize(glm::cross(forwardDir, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 upDir      = glm::vec3(0.0f, 1.0f, 0.0f); // Pure world-up for E/Q vertical movement

            float currentSpeed = moveSpeed * deltaTime;

            glm::vec3 pos(0,0,0);

            if (keyboardState[SDL_SCANCODE_W]) {
                pos += forwardDir * currentSpeed;
            }
            if (keyboardState[SDL_SCANCODE_S]) {
                pos -= forwardDir * currentSpeed;
            }
            if (keyboardState[SDL_SCANCODE_A]) {
                pos += rightDir * currentSpeed;
            }
            if (keyboardState[SDL_SCANCODE_D]) {
                pos -= rightDir * currentSpeed;
            }

            if (keyboardState[SDL_SCANCODE_E]) {
                pos += upDir * currentSpeed;
            }
            if (keyboardState[SDL_SCANCODE_Q]) {
                pos -= upDir * currentSpeed;
            }

        SetPosition(GetPosition() + pos);

    }

};
#endif //VOXEL_RAYTRACER_CAMERA_H