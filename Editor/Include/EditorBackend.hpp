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

        static void Update();
        static void Begin(std::string_view name);
        static void End();

        static void BeginStatsBar(float height);
        static void EndStatsBar();

        static void StatsBarEntry(const std::function<void()>& func);

        static void PushFont(int index);
        static void PopFont();

        static void Image(RenderTargetHandle renderTargetHandle);
        static void Text(std::string_view text);

        void Inspect(std::string_view name, int* value) const;
        void Inspect(std::string_view name, float* value) const;

    private:
        void SetupStyle() const;
    };
} // Neo
