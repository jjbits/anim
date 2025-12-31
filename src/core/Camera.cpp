#include "Camera.hpp"

#include <algorithm>

using namespace glm;

namespace anim::core {

Camera::Camera(vec3 position, float yaw, float pitch)
    : position_(position)
    , worldUp_(0.0f, 1.0f, 0.0f)
    , yaw_(yaw)
    , pitch_(pitch) {
    updateVectors();
}

mat4 Camera::viewMatrix() const {
    return lookAt(position_, position_ + front_, up_);
}

void Camera::moveForward(float delta) {
    position_ += front_ * delta * moveSpeed_;
}

void Camera::moveRight(float delta) {
    position_ += right_ * delta * moveSpeed_;
}

void Camera::moveUp(float delta) {
    position_ += worldUp_ * delta * moveSpeed_;
}

void Camera::rotate(float deltaYaw, float deltaPitch) {
    yaw_ += deltaYaw * sensitivity_;
    pitch_ += deltaPitch * sensitivity_;

    // Clamp pitch to avoid flipping
    pitch_ = std::clamp(pitch_, -89.0f, 89.0f);

    updateVectors();
}

void Camera::updateVectors() {
    vec3 front;
    front.x = cos(radians(yaw_)) * cos(radians(pitch_));
    front.y = sin(radians(pitch_));
    front.z = sin(radians(yaw_)) * cos(radians(pitch_));
    front_ = normalize(front);

    right_ = normalize(cross(front_, worldUp_));
    up_ = normalize(cross(right_, front_));
}

} // namespace anim::core
