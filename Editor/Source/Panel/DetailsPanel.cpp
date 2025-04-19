#include "Panel/DetailsPanel.hpp"
#include "Editor.hpp"
#include "misc/cpp/imgui_stdlib.h"


namespace Neo
{
    void DetailsPanel::Draw()
    {
        const auto& backend = Editor().Backend();
        backend.Begin("Details");
        const auto entity = Editor().GetSelectedEntity();
        if (entity == entt::null)
        {
            backend.End();
            return;
        }

        static std::unordered_set typesToSkip = {entt::type_hash<Hierarchy>::value()};
        for (auto [id, storage] : Engine.ECS().GetWorld().storage())
        {
            if (typesToSkip.contains(storage.type().hash())) continue;
            if (!storage.contains(Editor().GetSelectedEntity())) continue;

            auto type = resolve(storage.type());
            if (!type) continue;

            auto name = std::string(type.info().name().data());
            name = ReplaceString(name, "struct Neo::Component::", "");
            name = ReplaceString(name, "struct Component::", "");
            name = ReplaceString(name, "struct ", "");
            name = ReplaceString(name, ">(void) noexcept", "");

            if (!ImGui::CollapsingHeader(name.c_str())) continue;


            for (const auto& [id, metaData] : type.data())
            {
                auto instance = type.from_void(storage.value(entity));

                if (Resolve<int>(metaData, instance, "int",
                                 [](std::string_view memberName, auto& val)
                                 {
                                     return ImGui::DragInt(memberName.data(), val);
                                 }))
                    continue;

                if (Resolve<i64>(metaData, instance, "i64",
                                 [](std::string_view memberName, auto& val)
                                 {
                                     return ImGui::DragScalar(memberName.data(), ImGuiDataType_S64, val);
                                 }))
                    continue;

                if (Resolve<u8>(metaData, instance, "u8",
                                [](std::string_view memberName, auto& val)
                                {
                                    return ImGui::DragScalar(memberName.data(), ImGuiDataType_U8, val);
                                }))
                    continue;

                if (Resolve<u16>(metaData, instance, "u16",
                                 [](std::string_view memberName, auto& val)
                                 {
                                     return ImGui::DragScalar(memberName.data(), ImGuiDataType_U16, val);
                                 }))
                    continue;

                if (Resolve<u32>(metaData, instance, "u32",
                                 [](std::string_view memberName, auto& val)
                                 {
                                     return ImGui::DragScalar(memberName.data(), ImGuiDataType_U32, val);
                                 }))
                    continue;

                if (Resolve<u64>(metaData, instance, "u64",
                                 [](std::string_view memberName, auto& val)
                                 {
                                     return ImGui::DragScalar(memberName.data(), ImGuiDataType_U64, val);
                                 }))
                    continue;

                if (Resolve<float>(metaData, instance, "float",
                                   [](std::string_view memberName, auto& val)
                                   {
                                       return ImGui::DragFloat(memberName.data(), val);
                                   }))
                    continue;

                if (Resolve<double>(metaData, instance, "double",
                                    [](std::string_view memberName, auto& val)
                                    {
                                        return ImGui::DragScalar(memberName.data(), ImGuiDataType_Double, val);
                                    }))
                    continue;

                if (Resolve<std::string>(metaData, instance, "string",
                                         [](std::string_view memberName, auto& val)
                                         {
                                             return ImGui::InputText(memberName.data(), val);
                                         }))
                    continue;

                if (Resolve<glm::vec2>(metaData, instance, "vec2",
                                       [](const std::string_view memberName, auto& val)
                                       {
                                           return ImGui::DragFloat2(memberName.data(), glm::value_ptr(*val));
                                       }))
                    continue;

                if (Resolve<glm::vec3>(metaData, instance, "vec3",
                                       [](const std::string_view memberName, auto& val)
                                       {
                                           return ImGui::DragFloat3(memberName.data(), glm::value_ptr(*val));
                                       }))
                    continue;

                if (Resolve<glm::vec4>(metaData, instance, "vec4",
                                       [](const std::string_view memberName, auto& val)
                                       {
                                           return ImGui::DragFloat4(memberName.data(), glm::value_ptr(*val));
                                       }))
                    continue;
            }
        }

        backend.End();
    }
}
