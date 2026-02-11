#include "Window.hpp"

#include <stdexcept>
#include <utility>

using namespace std;

namespace anim::core {

Window::Window(const string& title, uint32_t width, uint32_t height)
    : windowWidth(width), windowHeight(height) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw runtime_error(string("Failed to initialize SDL: ") + SDL_GetError());
    }

    window = SDL_CreateWindow(
        title.c_str(),
        static_cast<int>(width),
        static_cast<int>(height),
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        SDL_Quit();
        throw runtime_error(string("Failed to create window: ") + SDL_GetError());
    }
}

Window::~Window() {
    if (window) {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
}

Window::Window(Window&& other) noexcept
    : window(other.window)
    , windowWidth(other.windowWidth)
    , windowHeight(other.windowHeight)
    , closeRequested(other.closeRequested) {
    other.window = nullptr;
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        if (window) {
            SDL_DestroyWindow(window);
        }
        window = other.window;
        windowWidth = other.windowWidth;
        windowHeight = other.windowHeight;
        closeRequested = other.closeRequested;
        other.window = nullptr;
    }
    return *this;
}

void Window::pollEvents() {
    // Reset per-frame input state
    inputState.mouseDeltaX = 0.0f;
    inputState.mouseDeltaY = 0.0f;
    inputState.scrollDelta = 0.0f;
    inputState.toggleCameraMode = false;
    inputState.toggleWireframe = false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                closeRequested = true;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE) {
                    closeRequested = true;
                }
                if (event.key.key == SDLK_W) inputState.forward = true;
                if (event.key.key == SDLK_S) inputState.backward = true;
                if (event.key.key == SDLK_A) inputState.left = true;
                if (event.key.key == SDLK_D) inputState.right = true;
                if (event.key.key == SDLK_SPACE) inputState.up = true;
                if (event.key.key == SDLK_LSHIFT) inputState.down = true;
                if (event.key.key == SDLK_TAB && !event.key.repeat) {
                    inputState.toggleCameraMode = true;
                }
                if (event.key.key == SDLK_I && !event.key.repeat) {
                    inputState.toggleWireframe = true;
                }
                break;

            case SDL_EVENT_KEY_UP:
                if (event.key.key == SDLK_W) inputState.forward = false;
                if (event.key.key == SDLK_S) inputState.backward = false;
                if (event.key.key == SDLK_A) inputState.left = false;
                if (event.key.key == SDLK_D) inputState.right = false;
                if (event.key.key == SDLK_SPACE) inputState.up = false;
                if (event.key.key == SDLK_LSHIFT) inputState.down = false;
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    inputState.leftMouseDown = true;
                }
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    inputState.rightMouseDown = true;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    inputState.leftMouseDown = false;
                }
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    inputState.rightMouseDown = false;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                // Always track motion when any mouse button is down
                if (inputState.leftMouseDown || inputState.rightMouseDown) {
                    inputState.mouseDeltaX += event.motion.xrel;
                    inputState.mouseDeltaY += event.motion.yrel;
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                // Scroll wheel or trackpad two-finger scroll
                inputState.scrollDelta += event.wheel.y;
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                windowWidth = static_cast<uint32_t>(event.window.data1);
                windowHeight = static_cast<uint32_t>(event.window.data2);
                resized = true;
                break;
        }
    }
}

VkSurfaceKHR Window::createSurface(VkInstance instance) const {
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) {
        throw runtime_error(string("Failed to create Vulkan surface: ") + SDL_GetError());
    }
    return surface;
}

vector<const char*> Window::getRequiredVulkanExtensions() const {
    uint32_t count = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&count);
    return vector<const char*>(extensions, extensions + count);
}

} // namespace anim::core
