#pragma once
#include "App/App.hpp"
#include "Core/Core.hpp"

int main()
{
    Neo::Engine.Init();
    const auto app = Neo::CreateApp();
    app->Init();
    while (Neo::Engine.Device().IsRunning())
    {
        app->Update();
        Neo::Engine.Update();
        app->EndFrame();
    }
    app->Shutdown();
    Neo::Engine.Shutdown();
}

