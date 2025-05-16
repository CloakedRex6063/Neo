#include "Core/Engine.hpp"
#include "Tools/Log.hpp"
#include "Core/Scripting.hpp"
#include "Project/ProjectBuilder.hpp"

namespace Neo
{
    EngineClass Engine;

    static auto time = std::chrono::high_resolution_clock::now();

    void EngineClass::Init()
    {
        Log::Init();
        mECS = new Neo::ECS();
        mDevice = new Neo::Device();
        mRenderer = new Neo::Renderer();
        mScripting = new Neo::Scripting();
        mResources = new Neo::Resources();
        mProject = new Neo::Project();

        // const ProjectInfo info
        // {
        //     .Name = "Sandbox",
        //     .Version = "0.0.1"
        // };
        // const auto path = std::filesystem::current_path().generic_string() + "/" + "Sandbox";
        //mProject->GenerateProject(info, path);
        mProject->LoadProject(std::filesystem::current_path().generic_string() + "/" + "Sandbox/Sandbox.proj");
    }

    void EngineClass::Shutdown() const
    {
        delete mDevice;
        delete mScripting;
        delete mECS;
        delete mProject;
        delete mRenderer;
    }

    void EngineClass::Update() 
    {
        const auto ctime = std::chrono::high_resolution_clock::now();
        const auto elapsed = ctime - time;
        mDeltaTime = std::chrono::duration<float>(elapsed).count();
        time = ctime;

        mDevice->Update(mDeltaTime);
        mScripting->Update(mDeltaTime);

        // Should always happen last
        mRenderer->Update(mDeltaTime);
        
        
        mDevice->ResetState();
    }
}
