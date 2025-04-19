#pragma once
#include "EditorBackend.hpp"
#include "Panel/IEditorPanel.hpp"

namespace Neo
{
    class DetailsPanel final : public IEditorPanel
    {
    public:
        explicit DetailsPanel(Neo::Editor& editor) : IEditorPanel(editor)
        {
        }

        void Draw() override;

    private:
        template <typename T, typename Func>
        bool Resolve(const entt::meta_data& metaData, entt::meta_any& instance, const std::string_view name,
                     Func&& func)
        {
            auto value = metaData.get(instance);
            if (!value) return false;

            if (entt::resolve(entt::hashed_string{name.data()}).id() != metaData.type().id()) return false;

            auto* val = value.try_cast<T>();
            if (!val) return false;

            auto stringName = std::string_view(*static_cast<std::string*>(metaData.custom()));
            if (!func(stringName, val)) return false;
            metaData.set(instance, *val);
            return true;
        };
    };
}
