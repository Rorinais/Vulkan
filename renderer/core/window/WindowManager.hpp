#pragma once
#include"../base.h"

class WindowManager {
public:
	struct Config{
		uint32_t width = 800;
		uint32_t height = 600;
		const char* title = "Vulkan App";
		bool resizable = false;
		int monitorIndex = 0; 
		bool fullScreen = false;
		bool highDPI = false;
	};
	using ResizeCallback = std::function<void(int, int)>;
    using KeyCallback = std::function<void(int key, int action)>;

    WindowManager(const Config& config);
    ~WindowManager();

    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;

    GLFWwindow* getHandle() const noexcept { return mWindow; }

    void setResizeCallback(ResizeCallback callback);
    void setKeyCallback(KeyCallback callback);

    bool shouldClose() const;
    void pollEvents() const;
    VkSurfaceKHR createSurface(VkInstance instance)const;

private:
    static void initGLFW();
    static void terminateGLFW();

private:
    GLFWwindow* mWindow = nullptr;
    Config mConfig;

    ResizeCallback mResizeCallback;
    KeyCallback mKeyCallback;

    static inline int s_glfwRefCount = 0;
};





