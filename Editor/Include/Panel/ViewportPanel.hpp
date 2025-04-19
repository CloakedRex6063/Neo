#pragma once

#include "EditorBackend.hpp"
#include "Panel/IEditorPanel.hpp"

namespace Neo
{
    class ViewportPanel final : public IEditorPanel
    {
    public:
        explicit ViewportPanel(Neo::Editor& editor) : IEditorPanel(editor) {}
        void Draw() override;
    };
}
