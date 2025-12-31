#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

using namespace std;

namespace anim::core {

struct InputState {
    // Keyboard
    bool forward = false;   // W
    bool backward = false;  // S
    bool left = false;      // A
    bool right = false;     // D
    bool up = false;        // Space
    bool down = false;      // Shift

    // Mouse
    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;
    bool leftMouseDown = false;   // Left click drag to rotate
    bool rightMouseDown = false;  // Right click drag to pan
};

class Window {
public:
    Window(const string& title, uint32_t width, uint32_t height);
    ~Window();

    // Non-copyable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Movable
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    void pollEvents();
    bool shouldClose() const { return closeRequested; }

    VkSurfaceKHR createSurface(VkInstance instance) const;
    vector<const char*> getRequiredVulkanExtensions() const;

    uint32_t width() const { return windowWidth; }
    uint32_t height() const { return windowHeight; }
    SDL_Window* handle() const { return window; }

    // Input
    const InputState& input() const { return inputState; }

private:
    SDL_Window* window = nullptr;
    uint32_t windowWidth = 0;
    uint32_t windowHeight = 0;
    bool closeRequested = false;

    InputState inputState;
};

} // namespace anim::core
