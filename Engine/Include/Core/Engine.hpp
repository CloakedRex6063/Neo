#pragma once
#include "Core/ECS.hpp"
#include "Core/Renderer.hpp"
#include "Core/Resources.hpp"
#include "Core/Device.hpp"
#include "Core/FileIO.hpp"

struct GLFWwindow;
namespace Neo
{
    struct EngineCreateInfo
    {
        HWND mHwnd;
        GLFWwindow* mWindow;
    };
    class Project;
    class Scripting;
    class EngineClass
    {
    public:
        void Init();
        void Update();
        void Shutdown() const;

        [[nodiscard]] Device& Device() const { return *mDevice; }
        [[nodiscard]] Renderer& Renderer() const { return *mRenderer; }
        [[nodiscard]] Scripting& Scripting() const { return *mScripting; }
        [[nodiscard]] Project& Project() const { return *mProject; }
        [[nodiscard]] Resources& Resources() const { return *mResources; }
        [[nodiscard]] ECS& ECS() const { return *mECS; }

        [[nodiscard]] float GetDeltaTime() const { return mDeltaTime; }

    private:
        Neo::Device* mDevice = nullptr;
        Neo::Renderer* mRenderer = nullptr;
        Neo::Scripting* mScripting = nullptr;
        Neo::Project* mProject = nullptr;
        Neo::Resources* mResources = nullptr;
        Neo::ECS* mECS = nullptr;
        float mDeltaTime = 0.f;
    };

    extern EngineClass Engine;
}
