#pragma once

namespace Neo
{
    class Editor;
    class IEditorPanel;
    template<class T>
    concept Panel = std::is_base_of_v<IEditorPanel, T>;
    
    class IEditorPanel
    {
    public:
        explicit IEditorPanel(Editor& editor) : mEditor(editor)
        {
        }

        virtual ~IEditorPanel() = default;
        virtual void Draw() = 0;

        Editor& Editor() const { return mEditor; }

    private:
        Neo::Editor& mEditor;
    };
}
