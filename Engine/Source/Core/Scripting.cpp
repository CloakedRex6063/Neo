#include "Core/Scripting.hpp"
#include "Core/Engine.hpp"
#include "Tools/Log.hpp"
#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"

namespace
{
    MonoDomain* mRootDomain = nullptr;
    MonoDomain* mAppDomain = nullptr;
    void LogInfo(MonoString* str)
    {
        Neo::Log::Info("{}", mono_string_to_utf8(str));
    }
    void LogWarn(MonoString* str)
    {
        Neo::Log::Warn("{}", mono_string_to_utf8(str));
    }
    void LogError(MonoString* str)
    {
        Neo::Log::Error("{}", mono_string_to_utf8(str));
    }
    
    MonoObject* object = nullptr;
}

namespace Neo
{
    Scripting::Scripting()
    {
        SetupMono();
        mono_add_internal_call("Internal::Log::Info", LogInfo);
    }

    Scripting::~Scripting()
    {
        mono_jit_cleanup(mRootDomain);
    }

    void Scripting::Update(float)
    {

    }

    void Scripting::RegisterMath()
    {
    }

    void Scripting::RegisterCore()
    {
    }

    void Scripting::SetupMono()
    {
        const auto currentPath = std::filesystem::current_path().generic_string();
        mono_set_dirs(currentPath.c_str(), currentPath.c_str());
        mRootDomain = mono_jit_init("MonoRuntime");
        if (!mRootDomain)
        {
            Log::Critical("Failed to initialise mono");
        }

        std::string name = "AppDomain";
        mAppDomain = mono_domain_create_appdomain(nullptr, nullptr);
        mono_domain_set(mAppDomain, true);
        if (!mAppDomain)
        {
            Log::Critical("Failed to initialise appdomain");
        }
    }

    MonoAssembly* Scripting::LoadAssembly(const std::string_view assemblyPath) const
    {
        const auto code = Engine.FileIO().ReadBinaryFile(FileIO::Directory::eNone, assemblyPath);
        MonoImageOpenStatus status;
        auto* monoData = const_cast<char*>(code.data());
        auto* image = mono_image_open_from_data_full(monoData, code.size(), 1, &status, 0);

        if (status != MONO_IMAGE_OK)
        {
            Log::Critical("Failed to load Sandbox.dll {}", mono_image_strerror(status));
        }

        auto* assembly = mono_assembly_load_from_full(image, assemblyPath.data(), &status, 0);
        mono_image_close(image);
        return assembly;
    }

    void Scripting::IterateAssembly(MonoAssembly* assembly, std::string_view debugName) const
    {
        MonoImage* image = mono_assembly_get_image(assembly);
        if (!image)
        {
            Log::Error("Failed to get assembly image");
        }

        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

        Log::Info("Iterating {} types in {}", numTypes, debugName);

        for (int i = 0; i < numTypes; ++i)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

            Log::Info("{}.{}", nameSpace, name);
            if (i == 0) continue;
            auto* klass = GetClassInAssembly(assembly, nameSpace, name);
            void* iter = nullptr;
            while (MonoMethod* method = mono_class_get_methods(klass, &iter))
            {
                const char* methodName = mono_method_get_name(method);
                Log::Info("Method: {}", methodName);
            }
        }
    }

    MonoClass* Scripting::GetClassInAssembly(MonoAssembly* assembly, const char* namespaceName,
                                             const char* className) const
    {
        auto* image = mono_assembly_get_image(assembly);
        auto* klass = mono_class_from_name(image, namespaceName, className);

        if (klass == nullptr)
        {
            Log::Error("Failed to get class from assembly");
            return nullptr;
        }

        return klass;
    }

    MonoObject* Scripting::Instantiate(MonoAssembly* assembly, const char* namespaceName, const char* className) const
    {
        auto* assemblyClass = GetClassInAssembly(assembly, namespaceName, className);
        auto* object = mono_object_new(mAppDomain, assemblyClass);
        mono_runtime_object_init(object);
        return object;
    }

    void Scripting::CallFunction(MonoObject* object, const std::string_view functionName, void* params) const
    {
        auto* method = mono_class_get_method_from_name(mono_object_get_class(object), functionName.data(), 1);
        mono_runtime_invoke(method, object, &params, nullptr);
    }
}
