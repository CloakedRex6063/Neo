#include "Core/Device.hpp"
#include "GLFW/glfw3.h"
#include "Tools/Tools.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

Neo::Device::Device()
{
    glfwInit();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    mWindow = glfwCreateWindow(static_cast<int>(static_cast<float>(mode->width) * 0.75f),
                               static_cast<int>(static_cast<float>(mode->height) * 0.75f), "Neo", nullptr,
                               nullptr);
    if (!mWindow)
    {
        Neo::ThrowError("Device::Device Failed to create window");
    }
    mHWND = glfwGetWin32Window(mWindow);
    glfwSetWindowUserPointer(mWindow, this);
    int width, height;
    glfwGetWindowSize(mWindow, &width, &height);
    mWindowSize.x = width;
    mWindowSize.y = height;
    glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, const int width, const int height)
    {
        const auto windowPointer = static_cast<Device*>(glfwGetWindowUserPointer(window));
        windowPointer->mWindowSize = {static_cast<float>(width), static_cast<float>(height)};
        windowPointer->mResized = true;
    });
}

Neo::Device::~Device()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

bool Neo::Device::IsRunning() const
{
    return !glfwWindowShouldClose(mWindow);
}

void Neo::Device::Update(float) const
{
    glfwPollEvents();
}

void Neo::Device::ResetState()
{
    mResized = false;
}
