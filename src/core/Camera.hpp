#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace anim::core {

enum class CameraMode {
    FPS,    // First-person: WASD movement, click+drag look
    Orbit   // Orbit around target: click+drag orbits, scroll zooms
};

class Camera {
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           float yaw = -90.0f, float pitch = 0.0f);

    // Get view matrix
    glm::mat4 viewMatrix() const;

    // Mode
    CameraMode mode() const { return mode_; }
    void setMode(CameraMode mode);
    void toggleMode();

    // FPS mode: movement
    void moveForward(float delta);
    void moveRight(float delta);
    void moveUp(float delta);

    // FPS mode: look (mouse input)
    void rotate(float deltaYaw, float deltaPitch);

    // Orbit mode: orbit around target
    void orbit(float deltaYaw, float deltaPitch);
    void zoom(float delta);
    void pan(float deltaX, float deltaY);

    // Orbit mode: target
    void setTarget(glm::vec3 target);
    glm::vec3 target() const { return target_; }

    // Accessors
    glm::vec3 position() const { return position_; }
    glm::vec3 front() const { return front_; }
    float yaw() const { return yaw_; }
    float pitch() const { return pitch_; }

    // Settings
    void setMoveSpeed(float speed) { moveSpeed_ = speed; }
    void setSensitivity(float sens) { sensitivity_ = sens; }
    void setZoomSpeed(float speed) { zoomSpeed_ = speed; }
    float moveSpeed() const { return moveSpeed_; }
    float sensitivity() const { return sensitivity_; }
    float zoomSpeed() const { return zoomSpeed_; }

private:
    void updateVectors();
    void updateOrbitPosition();

    CameraMode mode_ = CameraMode::FPS;

    // Shared state
    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 worldUp_;

    float yaw_;
    float pitch_;

    // Orbit mode state
    glm::vec3 target_{0.0f, 0.0f, 0.0f};
    float distance_ = 3.0f;
    float orbitYaw_ = -90.0f;
    float orbitPitch_ = 0.0f;

    // Settings
    float moveSpeed_ = 2.5f;
    float sensitivity_ = 0.1f;
    float zoomSpeed_ = 0.5f;
    float minDistance_ = 0.5f;
    float maxDistance_ = 100.0f;
};

} // namespace anim::core
