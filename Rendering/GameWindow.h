//
// Created by harish on 10/18/26.
//

#ifndef VOXEL_RAYTRACER_GAMEWINDOW_H
#define VOXEL_RAYTRACER_GAMEWINDOW_H
#include <iostream>
#include <memory>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>


struct SDL_WindowDeleter {
    void operator()(SDL_Window* w) const {
        if (w) SDL_DestroyWindow(w);
    }
};
class GameWindow {

private:
    int _width =1000,_height = 1000;
    std::unique_ptr<SDL_Window,SDL_WindowDeleter> _window;

public:
    GameWindow(int width, int height) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(std::string("SDL Init failed: ") + SDL_GetError());
        }

        if (!CreateNewWindow()) {
            throw std::runtime_error(std::string("SDL Window failed: ") + SDL_GetError());
        }
    }
    [[nodiscard]] SDL_Window* GetWindow() const { return _window.get(); }

    bool CreateNewWindow() {
        _window = std::unique_ptr<SDL_Window,SDL_WindowDeleter>(SDL_CreateWindow("SDLWindow", _width,_height,SDL_WINDOW_RESIZABLE));
        if (!_window) {
            return false;
        }
        return true;
    }

    ~GameWindow() {
        if (GetWindow())
            SDL_DestroyWindow(GetWindow());
    }
};


#endif //VOXEL_RAYTRACER_GAMEWINDOW_H