#pragma once

struct GLFWwindow;

namespace Neo
{
    class Device
    {
    public:
        Device();
        ~Device();

        [[nodiscard]] GLFWwindow* GetWindow() const { return mWindow; }
        [[nodiscard]] HWND GetNativeHandle() const { return mHWND; }
        [[nodiscard]] bool IsRunning() const;
        [[nodiscard]] glm::uvec2 GetWindowSize() const { return mWindowSize; }

        [[nodiscard]] bool IsWindowResized() const
        {
            return mResized;
        }

        void Update(float deltaTime) const;
        void ResetState();

    private:
        bool mResized = false;
        glm::uvec2 mWindowSize{};
        GLFWwindow* mWindow;
        HWND mHWND;
    };
}
