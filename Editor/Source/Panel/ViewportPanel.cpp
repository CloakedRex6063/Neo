#include "Panel/ViewportPanel.hpp"

#include <Editor.hpp>

#include "Core/Engine.hpp"
#include "MaterialIcons.h"

namespace Neo
{
    void ViewportPanel::Draw()
    {
        const auto& backend = Editor().Backend();
        backend.Begin(std::string("Viewport ") + material_icon_gamepad);
        backend.Image(Engine.Renderer().GetRenderTarget());
        backend.End();
    }
}
