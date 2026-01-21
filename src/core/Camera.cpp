#include "Camera.hpp"

#include <algorithm>

using namespace glm;

namespace anim::core {

Camera::Camera(vec3 position, float yaw, float pitch)
    : position_(position)
    , worldUp_(0.0f, 1.0f, 0.0f)
    , yaw_(yaw)
    , pitch_(pitch)
    , orbitYaw_(yaw)
    , orbitPitch_(pitch) {
    updateVectors();
    // Initialize orbit distance from initial position
    distance_ = length(position_);
    if (distance_ < minDistance_) distance_ = 3.0f;
}

mat4 Camera::viewMatrix() const {
    return lookAt(position_, position_ + front_, up_);
}

void Camera::setMode(CameraMode mode) {
    if (mode_ == mode) return;

    if (mode == CameraMode::Orbit) {
        // Switching to orbit: compute orbit parameters from current position
        vec3 toCamera = position_ - target_;
        distance_ = length(toCamera);
        if (distance_ < minDistance_) distance_ = minDistance_;

        // Compute orbit angles from direction
        vec3 dir = normalize(toCamera);
        orbitPitch_ = degrees(asin(dir.y));
        orbitYaw_ = degrees(atan2(dir.z, dir.x));
    } else {
        // Switching to FPS: compute yaw/pitch from current look direction
        yaw_ = degrees(atan2(front_.z, front_.x));
        pitch_ = degrees(asin(front_.y));
        updateVectors();
    }

    mode_ = mode;
}

void Camera::toggleMode() {
    setMode(mode_ == CameraMode::FPS ? CameraMode::Orbit : CameraMode::FPS);
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

void Camera::orbit(float deltaYaw, float deltaPitch) {
    orbitYaw_ += deltaYaw * sensitivity_;
    orbitPitch_ += deltaPitch * sensitivity_;

    updateOrbitPosition();
}

void Camera::zoom(float delta) {
    distance_ -= delta * zoomSpeed_;
    distance_ = std::clamp(distance_, minDistance_, maxDistance_);

    updateOrbitPosition();
}

void Camera::pan(float deltaX, float deltaY) {
    // Pan moves both camera and target in screen space
    vec3 offset = right_ * (-deltaX * sensitivity_ * 0.1f) +
                  up_ * (deltaY * sensitivity_ * 0.1f);
    target_ += offset;

    updateOrbitPosition();
}

void Camera::setTarget(glm::vec3 target) {
    target_ = target;
    if (mode_ == CameraMode::Orbit) {
        updateOrbitPosition();
    }
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

void Camera::updateOrbitPosition() {
    // Compute camera position on sphere around target
    float x = cos(radians(orbitYaw_)) * cos(radians(orbitPitch_));
    float y = sin(radians(orbitPitch_));
    float z = sin(radians(orbitYaw_)) * cos(radians(orbitPitch_));

    vec3 direction(x, y, z);
    position_ = target_ + direction * distance_;

    // Camera looks at target
    front_ = normalize(target_ - position_);

    // Determine if camera is upside down based on pitch
    float normalizedPitch = fmod(fmod(orbitPitch_, 360.0f) + 360.0f, 360.0f);
    bool upsideDown = (normalizedPitch > 90.0f && normalizedPitch < 270.0f);

    // Flip up vector when upside down to prevent view flip
    vec3 up = upsideDown ? -worldUp_ : worldUp_;
    right_ = normalize(cross(front_, up));
    up_ = normalize(cross(right_, front_));
}

} // namespace anim::core
