#include"window.hpp"
static void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

void Window::terminateGLFW() {
    glfwTerminate();
    glfwSetErrorCallback(nullptr);
}

Window::Window(const Config& config) : mConfig(config) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }
    glfwSetErrorCallback(glfwErrorCallback);

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
    glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
        auto* manager = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (manager && manager->mResizeCallback) {
            manager->mResizeCallback(width, height);

            glfwPostEmptyEvent();
        }
        });
    glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* manager = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (manager && manager->mKeyCallback) {
            manager->mKeyCallback(key, action);
        }
        });
}

Window::~Window() {
    if (mWindow) {
        glfwDestroyWindow(mWindow);
    }
    terminateGLFW();
}

void Window::setResizeCallback(ResizeCallback callback) {
    mResizeCallback = callback;
}
void Window::setKeyCallback(KeyCallback callback) {
    mKeyCallback = callback;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(mWindow);
}

void Window::pollEvents() const {
    glfwPollEvents();
}
VkSurfaceKHR Window::createSurface(VkInstance instance) const {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (glfwCreateWindowSurface(instance, mWindow, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
    return surface;
}
