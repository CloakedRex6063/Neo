#include "App/AppImpl.hpp"
#include "Panel/OutlinerPanel.hpp"
#include "Panel/DetailsPanel.hpp"
#include "Editor.hpp"
#include "EditorBackend.hpp"
#include "Panel/IEditorPanel.hpp"
#include "Panel/ViewportPanel.hpp"
#include "MaterialIcons.h"
#include "Tools/Importer.hpp"

namespace Neo
{
    void Editor::Init()
    {
        mBackend = new EditorBackend();
        AddPanel<ViewportPanel>();
        AddPanel<DetailsPanel>();
        AddPanel<OutlinerPanel>();
        [[maybe_unused]]
            const auto entity = Engine.ECS().CreateEntity("Hello");
        const auto child = Engine.ECS().CreateEntity("Hello2");
        Engine.ECS().AddChild(entity, child);
        mSelectedEntity = entity;
        Importer::ImportGLTF(FileIO::GetPath(Location::eProject, "Assets/Models/Drift/Drift.gltf"),
                             FileIO::GetPath(Location::eProject, "Assets/Models/Drift/"));
    }

    void Editor::Update()
    {
        mBackend->Update();

        mBackend->StatsBarEntry([&]
        {
            const auto data = FormatString("{} {:07.02f}", material_icon_desktop_windows,
                                           1.f / Engine.GetDeltaTime());
            mBackend->Text(data);
        });

        for (const auto& panel : mPanels)
        {
            panel->Draw();
        }
    }

    void Editor::EndFrame()
    {
        ImGui::EndFrame();
    }

    void Editor::Shutdown()
    {
        delete mBackend;
    }
}

CREATE_APP(Neo::Editor);
