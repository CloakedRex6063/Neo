#pragma once
#include "mono/metadata/image.h"

namespace Neo
{
    class Scripting
    {
    public:
        Scripting();
        ~Scripting();

        void Update(float deltaTime);

    private:
        void RegisterMath();
        void RegisterCore();
        void SetupMono();
        [[nodiscard]] static MonoAssembly* LoadAssembly(std::string_view assemblyPath);
        void IterateAssembly(MonoAssembly* assembly, std::string_view debugName) const;
        [[nodiscard]] MonoClass* GetClassInAssembly(MonoAssembly* assembly, const char* namespaceName, const char* className) const;
        [[nodiscard]] MonoObject* Instantiate(MonoAssembly* assembly, const char* namespaceName, const char* className) const;
        void CallFunction(MonoObject* object, std::string_view functionName, void* params) const;
    };
}
