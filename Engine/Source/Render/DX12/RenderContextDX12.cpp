#include "Render/DX12/RenderContextDX12.hpp"
#include "Core/Device.hpp"
#include "Core/Engine.hpp"
#include "Render/DX12/HelpersDX12.hpp"
#include "dxgidebug.h"

namespace Neo {
    RenderContextDX12::RenderContextDX12(const RenderContextCreateInfo &args) : mArgs(args) {
        ChooseGPU();
        CreateDevice();
        CreateQueues();
        CreateSwapchain();
        CreateDescriptorHeaps();
        CreateFrameData();
        CreateHeaps();
        CreateRootSignature();
        CreateFences();
    }

    RenderContextDX12::~RenderContextDX12() { WaitForGPU(); }

    RenderTargetHandle RenderContextDX12::CreateRenderTarget(const RenderTargetCreateInfo createInfo,
                                                             const std::string_view debugName) {
        DX12::RenderTarget rt;
        rt.ResourceHandle = createInfo.ResourceHandle;
        const TextureCreateInfo textureCreateInfo{
            .Width = createInfo.Size.x,
            .Height = createInfo.Size.y,
            .Depth = 1,
            .MipLevels = 1,
            .Format = createInfo.Format,
            .ViewType = createInfo.ViewType,
            .RenderTarget = true,
        };
        if (rt.ResourceHandle == ResourceHandle::eNull) {
            rt.ResourceHandle = CreateResource(mTextureHeap, textureCreateInfo, debugName);
        }
        rt.RenderDescriptor = CreateRenderTargetView(rt.ResourceHandle, createInfo);
        rt.TextureDescriptor = CreateShaderResourceView(rt.ResourceHandle, textureCreateInfo);
        const auto handle = static_cast<RenderTargetHandle>(mRenderTargets.size());
        mRenderTargets.emplace_back(rt);
        return handle;
    }

    DepthStencilHandle RenderContextDX12::CreateDepthStencil(DepthStencilCreateInfo createInfo,
                                                             std::string_view debugName) {
        const auto handle = static_cast<DepthStencilHandle>(mDepthStencils.size());
        mDepthStencils.emplace_back();
        return handle;
    }

    void RenderContextDX12::DestroyRenderTarget(RenderTargetHandle renderTargetHandle) {
        const auto &renderTarget = mRenderTargets.at(static_cast<u32>(renderTargetHandle));
        const auto& [BaseResource, ResourceState] = mResources.at(static_cast<u32>(renderTarget.ResourceHandle));
        mRTVAllocator.Free(renderTarget.RenderDescriptor);
        mCBVUAVSRVAllocator.Free(renderTarget.TextureDescriptor);
        BaseResource->Release();
    }

    void RenderContextDX12::DestroyDepthStencil(DepthStencilHandle depthStencilHandle) {
        const auto &depthStencil = mDepthStencils.at(static_cast<u32>(depthStencilHandle));
        const auto& [BaseResource, ResourceState] = mResources.at(static_cast<u32>(depthStencil.ResourceHandle));
        mDSVAllocator.Free(depthStencil.DepthDescriptor);
        mCBVUAVSRVAllocator.Free(depthStencil.TextureDescriptor);
        BaseResource->Release();
    }

    void RenderContextDX12::DestroyBuffer(BufferHandle bufferHandle) {
        const auto &buffer = mBuffers.at(static_cast<u32>(bufferHandle));
        const auto& [BaseResource, ResourceState] = mResources.at(static_cast<u32>(buffer.ResourceHandle));
        mCBVUAVSRVAllocator.Free(buffer.Descriptor);
        BaseResource->Release();
    }

    CommandHandle RenderContextDX12::CreateCommand(const QueueType queueType, std::string_view debugName) {
        D3D12_COMMAND_LIST_TYPE commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
        switch (queueType) {
            case QueueType::eGraphics:
                commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
                break;
            case QueueType::eTransfer:
                commandListType = D3D12_COMMAND_LIST_TYPE_COPY;
                break;
        }
        DX12::Command command;
        const auto allocResult =
                mDevice->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&command.CommandAllocator));
        DX12::ThrowIfFailed(allocResult, "RenderContextDX12::CreateCommand Failed to create command allocator");
        const auto listResult = mDevice->CreateCommandList(
            0, commandListType, command.CommandAllocator, nullptr, IID_PPV_ARGS(&command.CommandList));
        DX12::ThrowIfFailed(listResult, "RenderContextDX12::CreateCommand Failed to create command list");
        mCommands.emplace_back(command);
        const auto closeResult = mCommands.back().CommandList->Close();
        DX12::ThrowIfFailed(closeResult, "RenderContextDX12::CreateCommand Failed to close command list");

        const auto wDebugName = std::wstring(debugName.begin(), debugName.end());
        const auto nameAllocResult = command.CommandAllocator->SetName(wDebugName.c_str());
        DX12::ThrowIfFailed(nameAllocResult, "RenderContextDX12::CreateCommand Failed to name command allocator");

        const auto nameListResult = command.CommandList->SetName(wDebugName.c_str());
        DX12::ThrowIfFailed(nameListResult, "RenderContextDX12::CreateCommand Failed to name command list");

        return static_cast<CommandHandle>(mCommands.size() - 1);
    }

    BufferHandle RenderContextDX12::CreateBuffer(const BufferCreateInfo &createInfo, const std::string_view debugName) {
        DX12::Buffer buffer{};

        buffer.ResourceHandle = createInfo.ResourceHandle;
        if (createInfo.ResourceHandle == ResourceHandle::eNull) {
            DX12::Heap heap;
            switch (createInfo.Type) {
                case BufferType::eStorage:
                    heap = mBufferHeap;
                case BufferType::eUniform:
                    break;
                case BufferType::eStaging:
                    heap = mUploadHeap;
                    break;
                case BufferType::eReadback:
                    heap = mReadbackHeap;
                    break;
            }
            buffer.ResourceHandle = CreateResource(heap, createInfo, debugName);
        }
        buffer.Descriptor = CreateShaderResourceView(buffer.ResourceHandle, createInfo);

        const auto handle = static_cast<BufferHandle>(mBuffers.size());
        mBuffers.emplace_back(buffer);
        return handle;
    }

    ShaderHandle RenderContextDX12::CreateShader(const GraphicsShaderCreateInfo &createInfo,
                                                 std::string_view debugName) {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{
            .pRootSignature = mRootSignature,
            .VS =
            {
                .pShaderBytecode = createInfo.VertexCode.data(),
                .BytecodeLength = createInfo.VertexCode.size(),
            },
            .PS =
            {
                .pShaderBytecode = createInfo.FragmentCode.data(),
                .BytecodeLength = createInfo.FragmentCode.size(),
            },
            .BlendState = CD3DX12_BLEND_DESC{D3D12_DEFAULT},
            .SampleMask = D3D12_DEFAULT_SAMPLE_MASK,
            .RasterizerState =
            {
                .FillMode = DX12::GetFillMode(createInfo.FillMode),
                .CullMode = DX12::GetCullMode(createInfo.CullMode),
                .FrontCounterClockwise = DX12::GetFrontFace(createInfo.FrontFace),
            },
            .DepthStencilState =
            {
                .DepthEnable = createInfo.DepthStencilFormat != Format::eUnknown,
                .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
                .DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
            },
            .PrimitiveTopologyType = DX12::GetPrimitiveTopologyType(createInfo.PrimitiveTopology),
            .NumRenderTargets = createInfo.NumRenderTargets,
            .DSVFormat = DX12::GetFormat(createInfo.DepthStencilFormat),
            .SampleDesc = {.Count = 1, .Quality = 0},
            .Flags = D3D12_PIPELINE_STATE_FLAG_NONE,
        };

        for (int i = 0; i < createInfo.NumRenderTargets; i++) {
            desc.RTVFormats[i] = DX12::GetFormat(createInfo.RenderTargetFormats[i]);
        }
        ID3D12PipelineState *pipelineState;
        const auto result = mDevice->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState));
        DX12::ThrowIfFailed(result, "RenderContextDX12::CreateGraphicsShader Failed to create graphics pipeline");
        const auto wDebugName = std::wstring(debugName.begin(), debugName.end());
        const auto nameResult = pipelineState->SetName(wDebugName.c_str());
        DX12::ThrowIfFailed(nameResult, "RenderContextDX12::CreateGraphicsShader Failed to name compute pipeline");
        const auto shaderHandle = static_cast<ShaderHandle>(mShaders.size());
        mShaders.emplace_back(pipelineState);
        return shaderHandle;
    }

    ShaderHandle RenderContextDX12::CreateShader(const ComputeShaderCreateInfo &createInfo,
                                                 std::string_view debugName) {
        const D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {
            .pRootSignature = mRootSignature,
            .CS =
            {
                createInfo.ComputeCode.data(),
                createInfo.ComputeCode.size(),
            },
        };
        ID3D12PipelineState *pipelineState;
        const auto result = mDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pipelineState));
        DX12::ThrowIfFailed(result, "RenderContextDX12::CreateComputeShader Failed to create compute pipeline");
        const auto wDebugName = std::wstring(debugName.begin(), debugName.end());
        const auto nameResult = pipelineState->SetName(wDebugName.c_str());
        DX12::ThrowIfFailed(nameResult, "RenderContextDX12::CreateComputeShader Failed to name compute pipeline");
        const auto shaderHandle = static_cast<ShaderHandle>(mShaders.size());
        mShaders.emplace_back(pipelineState);
        return shaderHandle;
    }

    void *RenderContextDX12::MapBuffer(BufferHandle bufferHandle) {
        const auto &[Descriptor, Resource] = mBuffers.at(static_cast<u32>(bufferHandle));
        const auto &[BaseResource, ResourceState] = mResources.at(static_cast<u32>(Resource));

        constexpr D3D12_RANGE readRange = {0, 0};
        void *mappedPtr = nullptr;
        const auto result = BaseResource->Map(0, &readRange, &mappedPtr);
        DX12::ThrowIfFailed(result, "Failed to map buffer");
        return mappedPtr;
    }

    void RenderContextDX12::UnmapBuffer(BufferHandle bufferHandle) {
        const auto &[Descriptor, Resource] = mBuffers.at(static_cast<u32>(bufferHandle));
        const auto &[BaseResource, ResourceState] = mResources.at(static_cast<u32>(Resource));
        BaseResource->Unmap(0, nullptr);
    }

    u32 RenderContextDX12::GetGPUAddress(TextureHandle textureHandle) {
        const auto &[Descriptor, ResourceHandle] = mTextures.at(static_cast<u32>(textureHandle));
        return Descriptor.Index;
    }

    u32 RenderContextDX12::GetGPUAddress(BufferHandle bufferHandle) {
        const auto &[Descriptor, ResourceHandle] = mBuffers.at(static_cast<u32>(bufferHandle));
        return Descriptor.Index;
    }

    void RenderContextDX12::WaitForGPU() {
        const auto fenceValue = GetFrameData().FenceValue;
        const auto result = mGraphicsQueue->Signal(mGraphicsFence, fenceValue);
        DX12::ThrowIfFailed(result, "RenderContextDX12::WaitForGPU Failed to signal frame");
        const auto waitResult = mGraphicsFence->SetEventOnCompletion(fenceValue, nullptr);
        DX12::ThrowIfFailed(waitResult, "RenderContextDX12::WaitForGPU Failed to wait on this frame's work");
    }

    void RenderContextDX12::Resize() {
        WaitForGPU();
        for (const auto &frameData: mFrameDatas) {
            DestroyRenderTarget(frameData.RenderTargetHandle);
        }
        const auto size = Engine.Device().GetWindowSize();

        const auto result = mSwapchain->ResizeBuffers(
            kFrameCount, size.x, size.y, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
        DX12::ThrowIfFailed(result, "RenderContextDX12::Resize Failed to resize Swapchain");

        for (const auto &[index, frameData]: std::views::enumerate(mFrameDatas)) {
            const auto resource = DX12::GetSwapchainBuffer(mSwapchain, index);
            const auto debugName = std::string("Swapchain Buffer") + std::to_string(index);
            DX12::Name(resource, debugName);
            const auto resourceHandle = static_cast<ResourceHandle>(mResources.size());
            mResources.emplace_back(resource);

            const RenderTargetCreateInfo createInfo{
                .Size = Engine.Device().GetWindowSize(),
                .Format = Format::eB8G8R8A8_UNORM,
                .ViewType = ViewType::eTexture2D,
                .ResourceHandle = resourceHandle,
            };
            frameData.RenderTargetHandle = CreateRenderTarget(createInfo, debugName);
        }
        mFrameIndex = 0;
    }

    void RenderContextDX12::OneTimeSubmit(const std::vector<CommandHandle> &commandHandle, const QueueType queueType) {
        Submit(commandHandle, queueType);
        switch (queueType) {
            case QueueType::eGraphics: {
                const auto signalResult = mGraphicsQueue->Signal(mGraphicsFence, GetFrameData().FenceValue);
                DX12::ThrowIfFailed(signalResult, "RenderContextDX12::OneTimeSubmit Failed to signal graphics queue");
                const auto waitResult = mGraphicsFence->SetEventOnCompletion(GetFrameData().FenceValue, nullptr);
                DX12::ThrowIfFailed(waitResult, "RenderContextDX12::WaitForGPU Failed to wait on this frame's work");
                GetFrameData().FenceValue++;
                break;
            }

            case QueueType::eTransfer: {
                const auto signalResult = mTransferQueue->Signal(mTransferFence, mTransferFenceValue);
                DX12::ThrowIfFailed(signalResult, "RenderContextDX12::OneTimeSubmit Failed to signal transfer queue");
                const auto waitResult = mTransferFence->SetEventOnCompletion(mTransferFenceValue, nullptr);
                DX12::ThrowIfFailed(waitResult, "RenderContextDX12::WaitForGPU Failed to wait on transfer queue");
                mTransferFenceValue++;
                break;
            }
        }
    }

    void RenderContextDX12::BeginCommand(CommandHandle commandHandle) {
        const auto &[CommandAllocator, CommandList] = mCommands.at(static_cast<u32>(commandHandle));
        const auto allocResult = CommandAllocator->Reset();
        DX12::ThrowIfFailed(allocResult, "RenderContextDX12::BeginCommand Failed to reset command allocator");
        const auto listResult = CommandList->Reset(CommandAllocator, nullptr);
        DX12::ThrowIfFailed(listResult, "RenderContextDX12::BeginCommand Failed to reset command");
    }

    void RenderContextDX12::EndCommand(CommandHandle commandHandle) {
        const auto &renderTarget = mRenderTargets.at(static_cast<u32>(GetFrameData().RenderTargetHandle));
        TransitionResource(commandHandle, renderTarget.ResourceHandle, D3D12_RESOURCE_STATE_PRESENT);
        const auto &[CommandAllocator, CommandList] = mCommands.at(static_cast<u32>(commandHandle));
        const auto listResult = CommandList->Close();
        DX12::ThrowIfFailed(listResult, "RenderContextDX12::EndCommand Failed to close command list");
    }

    void RenderContextDX12::SetupGraphicsCommand(CommandHandle commandHandle) {
        const auto &[CommandAllocator, CommandList] = mCommands.at(static_cast<u32>(commandHandle));
        CommandList->SetGraphicsRootSignature(mRootSignature);
        CommandList->SetComputeRootSignature(mRootSignature);
        CommandList->SetDescriptorHeaps(1, &mCBVUAVSRVAllocator.Heap);
    }

    void RenderContextDX12::Submit(const std::vector<CommandHandle> &commandHandles, const QueueType queueType) {
        std::array<ID3D12CommandList *, 4> commands{};
        for (const auto &[index, commandHandle]: std::views::enumerate(commandHandles)) {
            const auto &[CommandAllocator, CommandList] = mCommands.at(static_cast<u32>(commandHandle));
            commands[index] = CommandList;
        }
        switch (queueType) {
            case QueueType::eGraphics:
                mGraphicsQueue->ExecuteCommandLists(commandHandles.size(), commands.data());
                break;
            case QueueType::eTransfer:
                mTransferQueue->ExecuteCommandLists(commandHandles.size(), commands.data());
                break;
        }
    }

    void RenderContextDX12::Present() {
        const auto presentResult = mSwapchain->Present(0, 0);
        DX12::ThrowIfFailed(presentResult, "RenderContextDX12::Present Failed to present");
        WaitForFrame();
    }

    void RenderContextDX12::BeginRenderPass(const CommandHandle commandHandle, const RenderPassInfo &renderPassInfo) {
        std::array<D3D12_RENDER_PASS_RENDER_TARGET_DESC, 8> renderTargetDescs{};
        for (const auto &[index, renderTargetIndex]: std::views::enumerate(renderPassInfo.RenderTargets)) {
            const auto &renderTarget = mRenderTargets.at(static_cast<u32>(renderTargetIndex));
            renderTargetDescs[index] = GetRenderTargetDesc(renderTarget,
                                                           renderPassInfo.RenderTargetLoadOp,
                                                           renderPassInfo.RenderTargetStoreOp,
                                                           renderPassInfo.ClearColor);
            TransitionResource(commandHandle, renderTarget.ResourceHandle, D3D12_RESOURCE_STATE_RENDER_TARGET);
        }

        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthStencilDesc = {};
        if (renderPassInfo.DepthStencil != DepthStencilHandle::eNull) {
            const auto &depthStencil = mDepthStencils.at(static_cast<u32>(renderPassInfo.DepthStencil));
            depthStencilDesc = GetDepthStencilDesc(depthStencil,
                                                   renderPassInfo.DepthStencilLoadOp,
                                                   renderPassInfo.DepthStencilStoreOp,
                                                   renderPassInfo.ClearDepth);
        }

        const auto &commandList = mCommands.at(static_cast<u32>(commandHandle)).CommandList;
        commandList->BeginRenderPass(renderPassInfo.RenderTargets.size(),
                                     renderTargetDescs.data(),
                                     renderPassInfo.DepthStencil != DepthStencilHandle::eNull
                                         ? &depthStencilDesc
                                         : nullptr,
                                     D3D12_RENDER_PASS_FLAG_NONE);
    }

    void RenderContextDX12::EndRenderPass(CommandHandle commandHandle) {
        const auto &commandList = mCommands.at(static_cast<u32>(commandHandle)).CommandList;
        commandList->EndRenderPass();
    }

    void RenderContextDX12::BindShader(CommandHandle commandHandle, ShaderHandle shaderHandle) {
        const auto &commandList = mCommands.at(static_cast<u32>(commandHandle)).CommandList;
        const auto &shader = mShaders.at(static_cast<u32>(shaderHandle));
        commandList->SetPipelineState(shader);
    }

    void RenderContextDX12::SetPrimitiveTopology(CommandHandle commandHandle, const PrimitiveTopology topology) {
        const auto &commandList = mCommands.at(static_cast<u32>(commandHandle)).CommandList;
        commandList->IASetPrimitiveTopology(DX12::GetPrimitiveTopology(topology));
    }

    void RenderContextDX12::Draw(CommandHandle commandHandle,
                                 const u32 vertexCount,
                                 const u32 instanceCount,
                                 const u32 vertexOffset,
                                 const u32 firstInstance) {
        const auto &commandList = mCommands.at(static_cast<u32>(commandHandle)).CommandList;
        commandList->DrawInstanced(vertexCount, instanceCount, vertexOffset, firstInstance);
    }

    void RenderContextDX12::DrawIndexed(CommandHandle commandHandle,
                                        const u32 indexCount,
                                        const u32 instanceCount,
                                        const u32 firstIndex,
                                        const int vertexOffset,
                                        const u32 firstInstance) {
        const auto &[CommandAllocator, CommandList] = mCommands.at(static_cast<u32>(commandHandle));
        CommandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void RenderContextDX12::SetViewport(CommandHandle commandHandle, const Viewport &viewport) {
        const auto &commandList = mCommands.at(static_cast<u32>(commandHandle)).CommandList;
        const D3D12_VIEWPORT dxViewport = {
            .TopLeftX = viewport.Offset.x,
            .TopLeftY = viewport.Offset.y,
            .Width = viewport.Dimensions.x,
            .Height = viewport.Dimensions.y,
            .MinDepth = viewport.DepthRange.x,
            .MaxDepth = viewport.DepthRange.y,
        };
        commandList->RSSetViewports(1, &dxViewport);
    }

    void RenderContextDX12::SetScissor(CommandHandle commandHandle, const Scissor &scissor) {
        const auto &commandList = mCommands.at(static_cast<u32>(commandHandle)).CommandList;
        const D3D12_RECT scissorRect = {
            .left = scissor.Min.x, .top = scissor.Min.y, .right = scissor.Max.x, .bottom = scissor.Max.y
        };
        commandList->RSSetScissorRects(1, &scissorRect);
    }

    void RenderContextDX12::BlitToSwapchain(CommandHandle commandHandle, RenderTargetHandle renderTargetHandle) {
        const auto &commandList = mCommands.at(static_cast<u32>(commandHandle)).CommandList;
        const auto &renderTarget = mRenderTargets.at(static_cast<u32>(renderTargetHandle));
        const auto &srcResource = mResources.at(static_cast<u32>(renderTarget.ResourceHandle));
        TransitionResource(commandHandle, renderTarget.ResourceHandle, D3D12_RESOURCE_STATE_COPY_SOURCE);

        const auto &swapchainRenderTarget = mRenderTargets.at(static_cast<u32>(GetFrameData().RenderTargetHandle));
        const auto &dstResource = mResources.at(static_cast<u32>(swapchainRenderTarget.ResourceHandle));
        TransitionResource(commandHandle, swapchainRenderTarget.ResourceHandle, D3D12_RESOURCE_STATE_COPY_DEST);

        commandList->CopyResource(dstResource.BaseResource, srcResource.BaseResource);
    }

    void RenderContextDX12::PushConstant(CommandHandle commandHandle, const u32 count, const void *data) {
        const auto &commandList = mCommands.at(static_cast<u32>(commandHandle)).CommandList;
        commandList->SetGraphicsRoot32BitConstants(0, count, data, 0);
    }

    void RenderContextDX12::ChooseGPU() {
        u32 flags = 0;
#ifdef NEO_DEBUG
        flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
        const auto result = CreateDXGIFactory2(flags, IID_PPV_ARGS(&mFactory));
        DX12::ThrowIfFailed(result, "Failed to create DXGIFactory2");
        Log::Info("Created DXGIFactory2");
        DXGI_GPU_PREFERENCE preference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
        switch (mArgs.GpuPreference) {
            case DevicePreference::eHighPerformance:
                preference = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
                break;
            case DevicePreference::ePowerSaving:
                preference = DXGI_GPU_PREFERENCE_MINIMUM_POWER;
                break;
        }

#ifdef NEO_DEBUG
        ID3D12Debug6* debugController;
        const auto debugResult = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
        DX12::ThrowIfFailed(debugResult, "RenderContextDX12::ChooseGPU Failed to get debug interface");
        debugController->EnableDebugLayer();
#endif
        IDXGIAdapter1 *adapter;
        for (UINT i = 0; mFactory->EnumAdapterByGpuPreference(i, preference, IID_PPV_ARGS(&adapter)) !=
                         DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

            if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)))
            {
                const auto queryResult = adapter->QueryInterface(IID_PPV_ARGS(&mAdapter));
                DX12::ThrowIfFailed(queryResult, "Failed to query adapter");
                adapter->Release();
                break;
            }
        }
        if (!mAdapter) {
            ThrowError("No suitable GPU found");
        }

        DXGI_ADAPTER_DESC3 desc{};
        const auto descResult = mAdapter->GetDesc3(&desc);
        DX12::ThrowIfFailed(descResult, "Failed to get Adapter description");

        NEO_WARN_BEG()
        NEO_WARN_WCONV()
        const auto description = std::string(std::begin(desc.Description), std::end(desc.Description));
        NEO_WARN_END()
        Log::Info("Adapter description:");
        Log::Info("{}", description);
        Log::Info("Shared Memory: {} GB", static_cast<float>(desc.SharedSystemMemory) / 1024 / 1024 / 1024);
        Log::Info("Dedicated Memory: {} GB", static_cast<float>(desc.DedicatedVideoMemory) / 1024 / 1024 / 1024);
        Log::Info("Total Memory: {} GB",
                  static_cast<float>(desc.SharedSystemMemory + desc.DedicatedVideoMemory) / 1024 / 1024 / 1024);
    }

    void RenderContextDX12::CreateDevice() {
        const auto deviceCreateResult = D3D12CreateDevice(mAdapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&mDevice));
        DX12::ThrowIfFailed(deviceCreateResult, "Failed to create device");
        Log::Info("Created device");

#ifdef NEO_DEBUG
        ID3D12InfoQueue1* infoQueue;
        const auto result = mDevice->QueryInterface(IID_PPV_ARGS(&infoQueue));
        DX12::ThrowIfFailed(result, "RenderContextDX12::CreateDevice Failed to create info queue");
        DWORD callbackCookie = 0;
        const auto registerResult = infoQueue->RegisterMessageCallback(
            [](D3D12_MESSAGE_CATEGORY,
               const D3D12_MESSAGE_SEVERITY Severity,
               D3D12_MESSAGE_ID,
               const LPCSTR pDescription,
               void*)
            {
                const auto description = std::string(pDescription);
                switch (Severity)
                {
                case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                    Log::Critical("[DX12] {}", description);
                    break;
                case D3D12_MESSAGE_SEVERITY_ERROR:
                    Log::Error("[DX12] {}", description);
                    break;
                case D3D12_MESSAGE_SEVERITY_WARNING:
                    Log::Warn("[DX12] {}", description);
                    break;
                case D3D12_MESSAGE_SEVERITY_INFO:
                case D3D12_MESSAGE_SEVERITY_MESSAGE:
                    break;
                }
            },
            D3D12_MESSAGE_CALLBACK_FLAG_NONE,
            nullptr,
            &callbackCookie);
        DX12::ThrowIfFailed(registerResult, "RenderContextDX12::CreateDevice Failed to register message callback");
#endif
    }

    void RenderContextDX12::CreateQueues() {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        };
        const auto graphicsResult = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mGraphicsQueue));
        DX12::ThrowIfFailed(graphicsResult, "Failed to create graphics queue");

        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
        const auto transferResult = mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mTransferQueue));
        DX12::ThrowIfFailed(transferResult, "Failed to create transfer queue");
    }

    void RenderContextDX12::CreateSwapchain() {
        const auto hwnd = Engine.Device().GetNativeHandle();
        const auto windowSize = Engine.Device().GetWindowSize();
        const DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
            .Width = windowSize.x,
            .Height = windowSize.y,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            .Stereo = false,
            .SampleDesc = {.Count = 1, .Quality = 0},
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 3,
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
            .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
        };
        IDXGISwapChain1 *swapChain;
        const auto createResult =
                mFactory->CreateSwapChainForHwnd(mGraphicsQueue, hwnd, &swapChainDesc, nullptr, nullptr, &swapChain);
        DX12::ThrowIfFailed(createResult, "Failed to create swap chain");
        const auto castResult = swapChain->QueryInterface(IID_PPV_ARGS(&mSwapchain));
        DX12::ThrowIfFailed(castResult, "Failed to get swap chain");
        Log::Info("Created swap chain");
    }

    void RenderContextDX12::CreateFrameData() {
        for (auto [index, frameData]: std::views::enumerate(mFrameDatas)) {
            frameData.CommandHandle = CreateCommand(QueueType::eGraphics, "Frame Command" + std::to_string(index));

            const auto resource = DX12::GetSwapchainBuffer(mSwapchain, index);
            const auto debugName = std::string("Swapchain Buffer") + std::to_string(index);
            DX12::Name(resource, debugName);
            const auto resourceHandle = static_cast<ResourceHandle>(mResources.size());
            mResources.emplace_back(resource);

            const RenderTargetCreateInfo createInfo{
                .Size = Engine.Device().GetWindowSize(),
                .Format = Format::eB8G8R8A8_UNORM,
                .ViewType = ViewType::eTexture2D,
                .ResourceHandle = resourceHandle,
            };
            frameData.RenderTargetHandle = CreateRenderTarget(createInfo, debugName);
        }
        mFrameIndex = mSwapchain->GetCurrentBackBufferIndex();
    }

    void RenderContextDX12::CreateDescriptorHeaps() {
        mRTVAllocator = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, "RTV Descriptor Heap");
        mDSVAllocator = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, "DSV Descriptor Heap");
        mCBVUAVSRVAllocator =
                CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "CBV_SRV_UAV Descriptor Heap");
    }

    void RenderContextDX12::CreateHeaps() {
        mBufferHeap = CreateHeap(D3D12_HEAP_TYPE_DEFAULT, 1 * 1024 * 1024 * 1024, "Buffer Heap");
        mTextureHeap = CreateHeap(D3D12_HEAP_TYPE_DEFAULT, 1 * 1024 * 1024 * 1024, "Texture Heap");
        mUploadHeap = CreateHeap(D3D12_HEAP_TYPE_UPLOAD, 200 * 1024 * 1024, "Upload Heap");
        mReadbackHeap = CreateHeap(D3D12_HEAP_TYPE_READBACK, 10 * 1024 * 1024, "Readback Heap");
    }

    void RenderContextDX12::CreateRootSignature() {
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;

        constexpr std::array parameters = {
            D3D12_ROOT_PARAMETER1{
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
                .Constants =
                {
                    .ShaderRegister = 0,
                    .RegisterSpace = 0,
                    .Num32BitValues = 58, // 58 because each root cbv takes 2 uints, and the max is 64
                },
            },
            D3D12_ROOT_PARAMETER1{
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor =
                {
                    .ShaderRegister = 1,
                    .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE,
                },
            },
            D3D12_ROOT_PARAMETER1{
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor =
                {
                    .ShaderRegister = 2,
                    .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE,
                },
            },
            D3D12_ROOT_PARAMETER1{
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor =
                {
                    .ShaderRegister = 3,
                    .Flags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE,
                },
            },
        };

        constexpr D3D12_STATIC_SAMPLER_DESC sampler = {
            // s_nearest
            .Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR,
            .AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            .MipLODBias = 0.0f,
            .MinLOD = 0.0f,
            .MaxLOD = D3D12_FLOAT32_MAX,
            .ShaderRegister = 0,
            .RegisterSpace = 0,
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL,
        };

        rootSignatureDesc.Init_1_1(parameters.size(),
                                   parameters.data(),
                                   1,
                                   &sampler,
                                   D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED);
        ID3DBlob *signature;
        ID3DBlob *error;
        const auto serializeResult = D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
        DX12::ThrowIfFailed(serializeResult, "Failed to serialize root signature");
        const auto signatureResult = mDevice->CreateRootSignature(
            0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
        DX12::ThrowIfFailed(signatureResult, "Failed to create root signature");
    }

    void RenderContextDX12::CreateFences() {
        const auto result = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mGraphicsFence));
        DX12::ThrowIfFailed(result, "Failed to create fence");
        const auto transferResult = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mTransferFence));
        DX12::ThrowIfFailed(transferResult, "Failed to create fence");
    }

    ResourceHandle RenderContextDX12::CreateResource(const DX12::Heap &heap,
                                                     const TextureCreateInfo &createInfo,
                                                     const std::string_view debugName) {
        D3D12_RESOURCE_FLAGS flags = {};
        D3D12_CLEAR_VALUE clearValue{};
        auto clearColor = glm::vec4(0, 0, 0, 0);
        constexpr float clearDepth = 1.0f;
        if (createInfo.RenderTarget) {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            clearValue = CD3DX12_CLEAR_VALUE{DX12::GetFormat(createInfo.Format), glm::value_ptr(clearColor)};
        } else if (createInfo.DepthStencil) {
            flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            clearValue = CD3DX12_CLEAR_VALUE{DXGI_FORMAT_R32_FLOAT, &clearDepth};
        }
        const D3D12_RESOURCE_DESC resourceDesc{
            .Dimension = DX12::GetResourceDimension(createInfo.ViewType),
            .Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
            .Width = createInfo.Width,
            .Height = createInfo.Height,
            .DepthOrArraySize = createInfo.Depth,
            .MipLevels = createInfo.MipLevels,
            .Format = DX12::GetFormat(createInfo.Format),
            .SampleDesc = {.Count = 1, .Quality = 0},
            .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags = flags,
        };
        ID3D12Resource2 *resource;
        const auto resourceResult = mDevice->CreatePlacedResource(heap.BaseHeap,
                                                                  heap.Offset,
                                                                  &resourceDesc,
                                                                  D3D12_RESOURCE_STATE_COMMON,
                                                                  &clearValue,
                                                                  IID_PPV_ARGS(&resource));
        DX12::Name(resource, debugName);
        DX12::ThrowIfFailed(resourceResult, "RenderContextDX12::CreateResource Failed to create texture resource");

        const auto handle = static_cast<ResourceHandle>(mResources.size());
        mResources.emplace_back(resource);
        return handle;
    }

    ResourceHandle
    RenderContextDX12::CreateResource(DX12::Heap &heap, const BufferCreateInfo &createInfo,
                                      std::string_view debugName) {
        const D3D12_RESOURCE_DESC resourceDesc{
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
            .Width = createInfo.NumElements * createInfo.Stride,
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_UNKNOWN,
            .SampleDesc = {.Count = 1, .Quality = 0},
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAG_NONE,
        };
        ID3D12Resource2 *resource;
        const auto resourceResult = mDevice->CreatePlacedResource(
            heap.BaseHeap, heap.Offset, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&resource));
        const auto wDebugName = std::wstring(debugName.begin(), debugName.end());
        DX12::ThrowIfFailed(resourceResult, "RenderContextDX12::CreateResource Failed to create buffer resource");

        const auto [SizeInBytes, Alignment] = mDevice->GetResourceAllocationInfo(0, 1, &resourceDesc);
        const u64 alignedOffset = DX12::Align(heap.Offset, Alignment);
        heap.Offset = alignedOffset + SizeInBytes;

        const auto handle = static_cast<ResourceHandle>(mResources.size());
        mResources.emplace_back(resource);
        return handle;
    }

    DX12::Heap
    RenderContextDX12::CreateHeap(const D3D12_HEAP_TYPE type, const u32 size, std::string_view debugName) const {
        D3D12_HEAP_PROPERTIES props;
        switch (type) {
            case D3D12_HEAP_TYPE_DEFAULT:
            default: {
                props = D3D12_HEAP_PROPERTIES{
                        .Type = D3D12_HEAP_TYPE_GPU_UPLOAD,
                    .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
                };
            }
                break;
            case D3D12_HEAP_TYPE_UPLOAD: {
                props = D3D12_HEAP_PROPERTIES{
                    .Type = D3D12_HEAP_TYPE_UPLOAD,
                    .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
                };
            }
            break;
            case D3D12_HEAP_TYPE_READBACK: {
                props = D3D12_HEAP_PROPERTIES{
                    .Type = D3D12_HEAP_TYPE_READBACK,
                    .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
                    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
                };
            }
        }
        const D3D12_HEAP_DESC desc = {
            .SizeInBytes = size,
            .Properties = props,
            .Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
            .Flags = D3D12_HEAP_FLAG_NONE,
        };
        DX12::Heap heap{};
        heap.Size = size;
        const auto result = mDevice->CreateHeap(&desc, IID_PPV_ARGS(&heap.BaseHeap));
        DX12::ThrowIfFailed(result, "Failed to create heap");

        DX12::Name(heap.BaseHeap, debugName);
        return heap;
    }

    DX12::DescriptorAllocator RenderContextDX12::CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE heapType,
                                                                      std::string_view debugName) const {
        bool shaderVisible = false;
        u32 numDescriptors = 0;
        switch (heapType) {
            case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
                shaderVisible = true;
                numDescriptors = 4096;
                break;
            case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
            case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
            default:
                shaderVisible = false;
                numDescriptors = 64;
                break;
        }

        DX12::DescriptorAllocator descriptorAllocator;

        const D3D12_DESCRIPTOR_HEAP_DESC desc = {
            .Type = heapType,
            .NumDescriptors = numDescriptors,
            .Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        };

        const auto result = mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorAllocator.Heap));
        DX12::ThrowIfFailed(result, "RenderContextDX12::CreateDescriptorHeap Failed to create descriptor heap");

        const auto wName = std::wstring(debugName.begin(), debugName.end());
        const auto nameResult = descriptorAllocator.Heap->SetName(wName.c_str());
        DX12::ThrowIfFailed(nameResult, "RenderContextDX12::CreateDescriptorHeap Failed to name descriptor heap");

        descriptorAllocator.CpuBase = descriptorAllocator.Heap->GetCPUDescriptorHandleForHeapStart();

        if (shaderVisible) {
            descriptorAllocator.GpuBase = descriptorAllocator.Heap->GetGPUDescriptorHandleForHeapStart();
        }

        descriptorAllocator.Stride = mDevice->GetDescriptorHandleIncrementSize(heapType);

        descriptorAllocator.Type = heapType;
        descriptorAllocator.At = 0;
        descriptorAllocator.Capacity = numDescriptors;
        return descriptorAllocator;
    }

    DX12::Descriptor RenderContextDX12::CreateShaderResourceView(const ResourceHandle resourceHandle,
                                                                 const BufferCreateInfo &createInfo) {
        const auto &[BaseResource, ResourceState] = mResources.at(static_cast<u32>(resourceHandle));
        const D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {
            .Format = DXGI_FORMAT_UNKNOWN,
            .ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Buffer = {
                .FirstElement = createInfo.FirstElement,
                .NumElements = createInfo.NumElements,
                .StructureByteStride = createInfo.Stride,
                .Flags = D3D12_BUFFER_SRV_FLAG_NONE,
            }
        };
        const auto srvDescriptor = mCBVUAVSRVAllocator.Allocate();
        mDevice->CreateShaderResourceView(BaseResource, &viewDesc, srvDescriptor.Cpu);
        return srvDescriptor;
    }

    DX12::Descriptor RenderContextDX12::CreateShaderResourceView(ResourceHandle resourceHandle,
                                                                 const TextureCreateInfo &createInfo) {
        const auto &[BaseResource, ResourceState] = mResources.at(static_cast<u32>(resourceHandle));
        const D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {
            .Format = DX12::GetFormat(createInfo.Format),
            .ViewDimension = DX12::GetSRVDimension(createInfo.ViewType),
            .Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2D = {
                .MostDetailedMip = 0,
                .MipLevels = createInfo.MipLevels,
                .PlaneSlice = 0,
                .ResourceMinLODClamp = 0,
            }
        };
        const auto srvDescriptor = mCBVUAVSRVAllocator.Allocate();
        mDevice->CreateShaderResourceView(BaseResource, &viewDesc, srvDescriptor.Cpu);
        return srvDescriptor;
    }

    DX12::Descriptor RenderContextDX12::CreateRenderTargetView(const ResourceHandle resourceHandle,
                                                               const RenderTargetCreateInfo &createInfo) {
        const D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {
            .Format = DX12::GetFormat(createInfo.Format),
            .ViewDimension = DX12::GetRTVDimension(createInfo.ViewType),
        };
        const auto rtvDescriptor = mRTVAllocator.Allocate();
        const auto &[BaseResource, ResourceState] = mResources.at(static_cast<u32>(resourceHandle));
        mDevice->CreateRenderTargetView(BaseResource, &renderTargetViewDesc, rtvDescriptor.Cpu);
        return rtvDescriptor;
    }

    void RenderContextDX12::TransitionResource(const CommandHandle commandHandle,
                                               const ResourceHandle resourceHandle,
                                               const D3D12_RESOURCE_STATES newState) {
        auto &[BaseResource, ResourceState] = mResources.at(static_cast<u32>(resourceHandle));
        if (ResourceState == newState)
            return;
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(BaseResource, ResourceState, newState);
        ResourceState = newState;
        mCommands.at(static_cast<u32>(commandHandle)).CommandList->ResourceBarrier(1, &barrier);
    }

    void RenderContextDX12::CopyBuffer(CommandHandle commandHandle,
                                       BufferHandle srcBufferHandle,
                                       BufferHandle dstBufferHandle,
                                       const u64 srcOffset,
                                       const u64 dstOffset,
                                       const u64 size) {
        const auto &command = mCommands.at(static_cast<u32>(commandHandle)).CommandList;

        const auto &[Descriptor, SrcResourceHandle] = mBuffers.at(static_cast<u32>(srcBufferHandle));
        const auto &[SrcResource, SrcState] = mResources.at(static_cast<u32>(SrcResourceHandle));

        TransitionResource(commandHandle, SrcResourceHandle, D3D12_RESOURCE_STATE_COPY_SOURCE);

        const auto &[DstDescriptor, DstResourceHandle] = mBuffers.at(static_cast<u32>(dstBufferHandle));
        const auto &[DstResource, DstState] = mResources.at(static_cast<u32>(DstResourceHandle));

        TransitionResource(commandHandle, DstResourceHandle, D3D12_RESOURCE_STATE_COPY_DEST);

        command->CopyBufferRegion(DstResource, dstOffset, SrcResource, srcOffset, size);
    }

    void RenderContextDX12::WaitForFrame() {
        auto &currentValue = GetFrameData().FenceValue;
        const auto result = mGraphicsQueue->Signal(mGraphicsFence, currentValue);
        DX12::ThrowIfFailed(result, "Failed to signal Queue");
        mFrameIndex = mSwapchain->GetCurrentBackBufferIndex();
        if (mGraphicsFence->GetCompletedValue() < currentValue) {
            const auto waitResult = mGraphicsFence->SetEventOnCompletion(currentValue, nullptr);
            DX12::ThrowIfFailed(waitResult, "Failed to wait for last frame");
        }
        currentValue++;
    }
} // namespace FS
