#pragma once
#include "Render/DX12/RenderStructsDX12.hpp"
#include "Render/RenderContext.hpp"

namespace Neo
{
    class RenderContextDX12 final : public IRenderContext
    {
    public:
        struct FrameData;
        void CreateFences();
        explicit RenderContextDX12(const RenderContextCreateInfo& args);
        ~RenderContextDX12() override;

        ResourceHandle GetResourceHandle(BufferHandle handle) override
        {
            return mBuffers.at(static_cast<u32>(handle)).ResourceHandle;
        }

        ResourceHandle GetResourceHandle(TextureHandle handle) override
        {
            return mTextures.at(static_cast<u32>(handle)).ResourceHandle;
        }

        ResourceHandle GetResourceHandle(DepthStencilHandle handle) override
        {
            return mDepthStencils.at(static_cast<u32>(handle)).ResourceHandle;
        }

        ResourceHandle GetResourceHandle(RenderTargetHandle handle) override
        {
            return mRenderTargets.at(static_cast<u32>(handle)).ResourceHandle;
        }

        [[nodiscard]] void* GetDevice() const override { return mDevice; }
        [[nodiscard]] void* GetGraphicsCommandQueue() const override { return mGraphicsQueue; }

        [[nodiscard]] void* GetGraphicsCommandList() override
        {
            return mCommands.at(static_cast<u32>(GetFrameData().CommandHandle)).CommandList;
        }

        [[nodiscard]] void* GetCBVUAVSRVAllocator() override { return &mCBVUAVSRVAllocator; }

        [[nodiscard]] void* GetRenderDescriptor(RenderTargetHandle renderTargetHandle) override
        {
            return &mRenderTargets.at(static_cast<u32>(renderTargetHandle)).RenderDescriptor;
        }

        [[nodiscard]] void* GetTextureDescriptor(RenderTargetHandle renderTargetHandle) override
        {
            return &mRenderTargets.at(static_cast<u32>(renderTargetHandle)).TextureDescriptor;
        }

        RenderTargetHandle CreateRenderTarget(RenderTargetCreateInfo createInfo, std::string_view debugName) override;
        DepthStencilHandle CreateDepthStencil(DepthStencilCreateInfo createInfo, std::string_view debugName) override;
        CommandHandle CreateCommand(QueueType queueType, std::string_view debugName) override;
        BufferHandle CreateBuffer(const BufferCreateInfo& createInfo, std::string_view debugName) override;
        ShaderHandle CreateShader(const GraphicsShaderCreateInfo& createInfo, std::string_view debugName) override;
        ShaderHandle CreateShader(const ComputeShaderCreateInfo& createInfo, std::string_view debugName) override;

        void DestroyRenderTarget(RenderTargetHandle renderTargetHandle) override;
        void DestroyDepthStencil(DepthStencilHandle depthStencilHandle) override;
        void DestroyBuffer(BufferHandle bufferHandle) override;

        void* MapBuffer(BufferHandle bufferHandle) override;
        void UnmapBuffer(BufferHandle bufferHandle) override;
        u32 GetGPUAddress(TextureHandle textureHandle) override;
        u32 GetGPUAddress(BufferHandle bufferHandle) override;
        void CopyBuffer(CommandHandle commandHandle,
                        BufferHandle srcBufferHandle,
                        BufferHandle dstBufferHandle,
                        u64 srcOffset,
                        u64 dstOffset,
                        u64 size) override;

        void WaitForFrame() override;
        void WaitForGPU() override;
        void Resize() override;
        void OneTimeSubmit(const std::vector<CommandHandle>& commandHandle, QueueType queueType) override;
        void Submit(const std::vector<CommandHandle>& commandHandles, QueueType queueType) override;
        void Present() override;
        void BeginCommand(CommandHandle commandHandle) override;
        void EndCommand(CommandHandle commandHandle) override;
        void SetupGraphicsCommand(CommandHandle commandHandle) override;
        void BeginRenderPass(CommandHandle commandHandle, const RenderPassInfo& renderPassInfo) override;
        void EndRenderPass(CommandHandle commandHandle) override;
        void BindShader(CommandHandle commandHandle, ShaderHandle shaderHandle) override;
        void SetPrimitiveTopology(CommandHandle commandHandle, PrimitiveTopology topology) override;
        void Draw(CommandHandle commandHandle,
                  u32 vertexCount,
                  u32 instanceCount,
                  u32 vertexOffset,
                  u32 firstInstance) override;
        void DrawIndexed(CommandHandle commandHandle,
                         u32 indexCount,
                         u32 instanceCount,
                         u32 firstIndex,
                         int vertexOffset,
                         u32 firstInstance) override;
        void SetViewport(CommandHandle commandHandle, const Viewport& viewport) override;
        void SetScissor(CommandHandle commandHandle, const Scissor& scissor) override;
        void BlitToSwapchain(CommandHandle commandHandle, RenderTargetHandle renderTargetHandle) override;
        void PushConstant(CommandHandle commandHandle, u32 count, const void* data) override;

    private:
        void ChooseGPU();
        void CreateDevice();
        void CreateQueues();
        void CreateSwapchain();
        void CreateFrameData();
        void CreateDescriptorHeaps();
        void CreateHeaps();
        void CreateRootSignature();
        [[nodiscard]] ResourceHandle
        CreateResource(const DX12::Heap& heap, const TextureCreateInfo& createInfo, std::string_view debugName);
        [[nodiscard]] ResourceHandle
        CreateResource(DX12::Heap& heap, const BufferCreateInfo& createInfo, std::string_view debugName);
        [[nodiscard]] DX12::Heap CreateHeap(D3D12_HEAP_TYPE type, u32 size, std::string_view debugName) const;
        [[nodiscard]] DX12::DescriptorAllocator CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType,
                                                                     std::string_view debugName) const;
        [[nodiscard]] DX12::Descriptor CreateShaderResourceView(ResourceHandle resourceHandle,
                                                                const BufferCreateInfo& createInfo);
        [[nodiscard]] DX12::Descriptor CreateShaderResourceView(ResourceHandle resourceHandle,
                                                                const TextureCreateInfo& createInfo);
        [[nodiscard]] DX12::Descriptor CreateRenderTargetView(ResourceHandle resourceHandle,
                                                              const RenderTargetCreateInfo& createInfo);
        void TransitionResource(CommandHandle commandHandle,
                                ResourceHandle resourceHandle,
                                D3D12_RESOURCE_STATES newState);

        RenderContextCreateInfo mArgs;
        IDXGIFactory7* mFactory;
        IDXGIAdapter4* mAdapter;
        IDXGISwapChain4* mSwapchain;
        ID3D12Device14* mDevice;
        ID3D12CommandQueue* mGraphicsQueue;
        ID3D12Fence1* mGraphicsFence;
        ID3D12CommandQueue* mTransferQueue;
        ID3D12Fence1* mTransferFence;
        u64 mTransferFenceValue = 0;
        ID3D12RootSignature* mRootSignature;

        DX12::DescriptorAllocator mCBVUAVSRVAllocator;
        DX12::DescriptorAllocator mRTVAllocator;
        DX12::DescriptorAllocator mDSVAllocator;

        DX12::Heap mBufferHeap;
        DX12::Heap mTextureHeap;
        DX12::Heap mUploadHeap;
        DX12::Heap mReadbackHeap;

        std::vector<DX12::Command> mCommands;
        std::vector<DX12::RenderTarget> mRenderTargets;
        std::vector<DX12::DepthStencil> mDepthStencils;
        std::vector<DX12::Texture> mTextures;
        std::vector<DX12::Buffer> mBuffers;
        std::vector<ID3D12PipelineState*> mShaders;
        std::vector<DX12::Resource> mResources;
    };
} // namespace FS
