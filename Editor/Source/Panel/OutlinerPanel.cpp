#include "Panel/OutlinerPanel.hpp"
#include "Editor.hpp"
#include "Core/Engine.hpp"

namespace Neo
{
    void OutlinerPanel::Draw()
    {
        const auto& backend = Editor().Backend();
        backend.Begin("Outliner");

        for (const auto entity : Engine.ECS().View<Entity>())
        {
            if (!Engine.ECS().Has<Name, Hierarchy>(entity)) continue;
            if (Engine.ECS().GetParent(entity) != NullEntity) continue;

            DrawEntityTree(entity);
        }

        backend.End();
    }

    void OutlinerPanel::DrawEntityTree(const Entity entity)
    {
        const bool selected = (Editor().GetSelectedEntity() == entity);
        ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding |
            ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        treeFlags |= selected ? ImGuiTreeNodeFlags_Selected : 0;
        auto name = Engine.ECS().GetName(entity);
        name = name.empty() ? "Unnamed" : name;

        const bool nodeOpen = ImGui::TreeNodeEx(name.data(), treeFlags);

        if (ImGui::IsItemClicked())
        {
            Editor().SetSelectedEntity(entity);
        }

        if (nodeOpen)
        {
            for (const auto& child : Engine.ECS().GetChildren(entity))
            {
                DrawEntityTree(child);
            }
            ImGui::TreePop();
        }
    }
}
