#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace anim::core {

class Camera {
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
           float yaw = -90.0f, float pitch = 0.0f);

    // Get view matrix
    glm::mat4 viewMatrix() const;

    // Movement
    void moveForward(float delta);
    void moveRight(float delta);
    void moveUp(float delta);

    // Look (mouse input)
    void rotate(float deltaYaw, float deltaPitch);

    // Accessors
    glm::vec3 position() const { return position_; }
    glm::vec3 front() const { return front_; }
    float yaw() const { return yaw_; }
    float pitch() const { return pitch_; }

    // Settings
    void setMoveSpeed(float speed) { moveSpeed_ = speed; }
    void setSensitivity(float sens) { sensitivity_ = sens; }
    float moveSpeed() const { return moveSpeed_; }
    float sensitivity() const { return sensitivity_; }

private:
    void updateVectors();

    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 worldUp_;

    float yaw_;
    float pitch_;

    float moveSpeed_ = 2.5f;
    float sensitivity_ = 0.1f;
};

} // namespace anim::core
