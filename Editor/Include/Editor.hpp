#pragma once
#include "Core/Core.hpp"
#include "App/App.hpp"

namespace Neo
{
    class Editor final : public App
    {
    public:
        void Init() override;
        void Update() override;
        void EndFrame() override;
        void Shutdown() override;

        EditorBackend& Backend() const { return *mBackend; }
        
        template <Panel T>
        void AddPanel()
        {
            mPanels.push_back(std::make_unique<T>(*this));
        }

        Entity GetSelectedEntity() const { return mSelectedEntity; }
        void SetSelectedEntity(const Entity entity) { mSelectedEntity = entity; }

    private:
        // Raw pointer for controlled deletion
        EditorBackend* mBackend = nullptr;
        std::vector<std::unique_ptr<IEditorPanel>> mPanels;
        Entity mSelectedEntity;
    };
}
