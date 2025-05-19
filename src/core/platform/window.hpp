#pragma once
#include"../../base.hpp"

class Window {
public:
    struct Config {
        uint32_t width = 800;
        uint32_t height = 600;
        const char* title = "Vulkan App";
        bool resizable = false;
        int monitorIndex = 0;
        bool fullScreen = false;
        bool highDPI = false;
    };

    using Ptr = std::shared_ptr<Window>;
    static Ptr create(const Window::Config& config) { return std::make_shared<Window>(config); }

    using ResizeCallback = std::function<void(int, int)>;
    using KeyCallback = std::function<void(int key, int action)>;

    Window(const Window::Config& config);
    ~Window();

    GLFWwindow* getHandle() const noexcept { return mWindow; }

    void setResizeCallback(ResizeCallback callback);
    void setKeyCallback(KeyCallback callback);

    bool shouldClose() const;
    void pollEvents() const;
    VkSurfaceKHR createSurface(VkInstance instance)const;

private:
    static void terminateGLFW();

private:
    GLFWwindow* mWindow = nullptr;
    Config mConfig;

    ResizeCallback mResizeCallback;
    KeyCallback mKeyCallback;
};





