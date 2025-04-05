#include "WindowManager.hpp"

static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void WindowManager::initGLFW() {
    if (s_glfwRefCount++ == 0) {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        glfwSetErrorCallback(glfwErrorCallback);
    }
}

void WindowManager::terminateGLFW() {
    if (--s_glfwRefCount == 0) {
        glfwTerminate();
        glfwSetErrorCallback(nullptr);
    }
}

WindowManager::WindowManager(const Config& config): mConfig(config){
    initGLFW();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); 
    glfwWindowHint(GLFW_RESIZABLE, mConfig.resizable ? GLFW_TRUE : GLFW_FALSE);
    if (config.highDPI) {
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    }

    GLFWmonitor* monitor = nullptr;
    const GLFWvidmode* mode = nullptr;
    int width = mConfig.width;
    int height = mConfig.height;

    if (config.fullScreen) {
        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount); 

        if (monitorCount == 0) {
            terminateGLFW();
            throw std::runtime_error("No monitors found");
        }

        if (config.monitorIndex >= 0 && config.monitorIndex < monitorCount) {
            monitor = monitors[config.monitorIndex];
            mode = glfwGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
        }
        else {
            terminateGLFW();
            throw std::runtime_error("Invalid monitor index: " + std::to_string(config.monitorIndex));
        }
    }

    mWindow = glfwCreateWindow(width, height, mConfig.title, monitor, nullptr);

    if (!mWindow) {
        terminateGLFW();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
        auto* manager = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
        if (manager && manager->mResizeCallback) {
            manager->mResizeCallback(width, height);
        }
     });
    glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* manager = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
        if (manager && manager->mKeyCallback) {
            manager->mKeyCallback(key, action);
        }
     });
}

WindowManager::~WindowManager() {
    if (mWindow) {
        glfwDestroyWindow(mWindow);
    }
    terminateGLFW();
}

void WindowManager::setResizeCallback(ResizeCallback callback) {
    mResizeCallback = callback;
}
void WindowManager::setKeyCallback(KeyCallback callback) {
    mKeyCallback = callback;
}

bool WindowManager::shouldClose() const {
    return glfwWindowShouldClose(mWindow);
}

void WindowManager::pollEvents() const {
    glfwPollEvents();
}
VkSurfaceKHR WindowManager::createSurface(VkInstance instance) const {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (glfwCreateWindowSurface(instance, mWindow, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
    return surface;
}
