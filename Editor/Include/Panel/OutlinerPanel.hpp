#pragma once 
#include "EditorBackend.hpp"
#include "Panel/IEditorPanel.hpp"

namespace Neo
{
    class OutlinerPanel final : public IEditorPanel
    {
    public:
        explicit OutlinerPanel(Neo::Editor& editor) : IEditorPanel(editor) {}
        void Draw() override;

    private:
        void DrawEntityTree(Entity entity);
    };
}
