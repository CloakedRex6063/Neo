#include "EditorBackend.hpp"
#include "Render/DX12/RenderContextDX12.hpp"
#include "Core/Engine.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_glfw.h"

namespace Neo
{
    EditorBackend::EditorBackend()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // We can guarantee its dx12 for this case, so a static_cast is faster
        auto* renderContext = static_cast<RenderContextDX12*>( // NOLINT(*-pro-type-static-cast-downcast)
            Engine.Renderer().GetRenderContext().get());

        ImGui_ImplDX12_InitInfo initInfo = {};
        initInfo.Device = static_cast<ID3D12Device*>(renderContext->GetDevice());
        initInfo.CommandQueue = static_cast<ID3D12CommandQueue*>(renderContext->GetGraphicsCommandQueue());
        initInfo.NumFramesInFlight = kFrameCount;
        initInfo.RTVFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
        initInfo.UserData = renderContext;
        initInfo.SrvDescriptorHeap = static_cast<DX12::DescriptorAllocator*>(renderContext->GetCBVUAVSRVAllocator())->
            Heap;
        // ReSharper disable once CppParameterMayBeConstPtrOrRef
        initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info,
                                           D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle,
                                           D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
        {
            const auto context = static_cast<IRenderContext*>(info->UserData);
            const auto descriptor = static_cast<DX12::DescriptorAllocator*>(context->GetCBVUAVSRVAllocator())->
                Allocate();
            out_cpu_handle->ptr = descriptor.Cpu.ptr;
            out_gpu_handle->ptr = descriptor.Gpu.ptr;
        };
        // ReSharper disable once CppParameterMayBeConstPtrOrRef
        initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info,
                                          const D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
                                          const D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
        {
            const auto context = static_cast<IRenderContext*>(info->UserData);
            static_cast<DX12::DescriptorAllocator*>(context->GetCBVUAVSRVAllocator())->Free(cpu_handle, gpu_handle);
        };
        ImGui_ImplDX12_Init(&initInfo);
        ImGui_ImplGlfw_InitForOther(Engine.Device().GetWindow(), true);
        SetupStyle();

        RenderPass renderPass
        {
            .Execute = []
            {
                const auto& context = Engine.Renderer().GetRenderContext();
                const auto& frameData = context->GetFrameData();
                const RenderPassInfo renderPassInfo{
                    .RenderTargets = {frameData.RenderTargetHandle},
                    .RenderTargetLoadOp = RenderPassInfo::LoadOp::eLoad,
                };
                context->BeginRenderPass(frameData.CommandHandle, renderPassInfo);
                const Viewport viewport{.Dimensions = Engine.Device().GetWindowSize()};
                context->SetViewport(frameData.CommandHandle, viewport);
                const Scissor scissor{.Max = Engine.Device().GetWindowSize()};
                context->SetScissor(frameData.CommandHandle, scissor);
                ImGui::Render();
                ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),
                                              static_cast<ID3D12GraphicsCommandList*>(context->
                                                  GetGraphicsCommandList()));
                context->EndRenderPass(frameData.CommandHandle);
            }
        };
        Engine.Renderer().AddRenderPass(std::move(renderPass));
    }

    EditorBackend::~EditorBackend()
    {
        Engine.Renderer().GetRenderContext()->WaitForGPU();
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorBackend::Update()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport();
    }

    void EditorBackend::Begin(const std::string_view name) { ImGui::Begin(name.data()); }

    void EditorBackend::End() { ImGui::End(); }

    void EditorBackend::BeginStatsBar(const float height)
    {
        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_MenuBar;

        ImGui::BeginViewportSideBar("##StatsBar", ImGui::GetMainViewport(), ImGuiDir_Down, height,
                                    windowFlags);
        ImGui::BeginMenuBar();
    }

    void EditorBackend::EndStatsBar()
    {
        ImGui::EndMenuBar();
        ImGui::End();
    }

    void EditorBackend::StatsBarEntry(const std::function<void()>& func)
    {
        BeginStatsBar(25.f);
        func();
        EndStatsBar();
    }

    void EditorBackend::PushFont(const int index)
    {
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[index]);
    }

    void EditorBackend::PopFont()
    {
        ImGui::PopFont();
    }

    void EditorBackend::Image(const RenderTargetHandle renderTargetHandle)
    {
        const auto descriptor = static_cast<DX12::Descriptor*>(Engine.Renderer().GetRenderContext()->
                                                                      GetTextureDescriptor(renderTargetHandle));
        ImGui::Image(descriptor->Gpu.ptr, ImGui::GetContentRegionAvail());
    }

    void EditorBackend::Text(const std::string_view text)
    {
        ImGui::Text(text.data());
    }

    void EditorBackend::Inspect(const std::string_view name, int* value) const
    {
        ImGui::InputInt(name.data(), value);
    }

    void EditorBackend::Inspect(const std::string_view name, float* value) const
    {
        ImGui::InputFloat(name.data(), value);
    }

    void EditorBackend::SetupStyle() const
    {
        ImVec4* colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.16f, 0.16f, 0.16f, 0.50f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
        colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
        colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
        colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
        colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(2.00f, 2.00f);
        style.FramePadding = ImVec2(2.00f, 2.00f);
        style.CellPadding = ImVec2(2.00f, 2.00f);
        style.ItemSpacing = ImVec2(6.00f, 6.00f);
        style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
        style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
        style.IndentSpacing = 25;
        style.ScrollbarSize = 15;
        style.GrabMinSize = 10;
        style.WindowBorderSize = 1;
        style.ChildBorderSize = 1;
        style.PopupBorderSize = 1;
        style.FrameBorderSize = 1;
        style.TabBorderSize = 1;
        style.WindowRounding = 7;
        style.ChildRounding = 4;
        style.FrameRounding = 3;
        style.PopupRounding = 4;
        style.ScrollbarRounding = 9;
        style.GrabRounding = 3;
        style.LogSliderDeadzone = 4;
        style.TabRounding = 4;

        const ImGuiIO& io = ImGui::GetIO();
        io.Fonts->Clear();
        ImFontConfig config;
        config.SizePixels = 20.0f;
        io.Fonts->AddFontFromFileTTF("Assets/JetbrainsMono.ttf", 20, &config);

        config.MergeMode = true;
        config.GlyphOffset.y = 3.5f;
        static constexpr uint16_t iconRanges[] = {0xE003, 0xF8FF, 0};
        io.Fonts->AddFontFromFileTTF(
            "Assets/MaterialSymbolsRounded.ttf",
            20, &config, iconRanges);
        io.Fonts->Build();
    }
} // Neo
