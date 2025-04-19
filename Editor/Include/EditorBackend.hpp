#pragma once
#include "imgui.h"
#include "Render/RenderStructs.hpp"

namespace Neo
{
    class EditorBackend
    {
    public:
        EditorBackend();
        ~EditorBackend();

        void Update() const;
        void Begin(std::string_view name) const;
        void End() const;

        void BeginStatsBar(float height) const;
        void EndStatsBar() const;

        void StatsBarEntry(const std::function<void()>& func) const;
        
        void PushFont(int index) const;
        void PopFont() const;
        
        void Image(RenderTargetHandle renderTargetHandle) const;
        void Text(std::string_view text) const;

        void Inspect(std::string_view name, int* value) const;
        void Inspect(std::string_view name, float* value) const;

    private:
        void SetupStyle() const;
    };
} // Neo
