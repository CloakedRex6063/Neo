#pragma once

namespace Neo
{
    class App
    {
    public:
        virtual ~App() = default;
        virtual void Init() = 0;
        virtual void Update() = 0;
        virtual void EndFrame() = 0;
        virtual void Shutdown() = 0;
    };

    extern App* CreateApp();
} // namespace FS

#define CREATE_APP(NAME)                                                       \
Neo::App* Neo::CreateApp()                                   \
{                                                                          \
return new NAME();                                       \
}
