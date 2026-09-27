//
// Created by harish on 9/27/26.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

class Object3D {
private:
    glm::vec3 _position;
    glm::quat _rotation;
    glm::mat4 _localMatrix;

    bool _isDirty = true;
public:
    Object3D() {

    }

    void SetPosition(const glm::vec3 pos) {
        _position = pos;
        _isDirty = true;
    }

    glm::vec3 GetPosition() {
        return _position;
    }

    void SetRotation(const glm::quat rotation) {
        _rotation = rotation;
    }

    void SetRotationEuler(const glm::vec3 eulerAngles) {
        glm::vec3 eulerAnglesRad = glm::radians(eulerAngles);
        _rotation = glm::quat( eulerAnglesRad);
        _isDirty = true;
    }

    glm::vec3 GetRotationEuler() {
        glm::vec3 eulerAngleRad = glm::eulerAngles( _rotation);
        return glm::degrees( eulerAngleRad);
    }

    glm::quat GetRotation() {
        return _rotation;
    }

    void RotateYawLocal(float degrees) {
        glm::quat delta = glm::angleAxis(glm::radians(degrees), glm::vec3(0.0f, 1.0f, 0.0f));
        _rotation = _rotation * delta;
        _isDirty = true;
    }

    void RotateRollLocal(float degrees) {
        glm::quat delta = glm::angleAxis(glm::radians(degrees), glm::vec3(0.0f, 0.0f, 1.0f));
        _rotation = _rotation * delta;
        _isDirty = true;
    }

    void RotatePitchLocal(float degrees) {
        glm::quat delta = glm::angleAxis(glm::radians(degrees), glm::vec3(1.0f, 0.0f, 0.0f));
        _rotation = _rotation * delta;
        _isDirty = true;
    }

    glm::vec3 GetForward() {
        return _rotation * glm::vec3(0.0f,0.0f, -1.0f);
    }
    glm::vec3 GetRight() {
        return _rotation * glm::vec3(1.0f,0.0f, 0.0f);
    }
    glm::vec3 GetUp() {
        return _rotation * glm::vec3(0.0f,1.0f, 0.0f);
    }

    glm::mat4 GetMatrix() {
        if (_isDirty) {
            glm::mat4 T = glm::translate( glm::mat4(1.0f), _position);
            glm::mat4 R = glm::mat4_cast( _rotation);

            _localMatrix = T * R;
            _isDirty = false;
        }

        return _localMatrix;
    }

    virtual void Tick(float deltaTime) = 0;
    virtual void Start() = 0;
};

#endif //VOXEL_RAYTRACER_CAMERA_H