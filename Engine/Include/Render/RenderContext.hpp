#pragma once
#include "Render/RenderConstants.hpp"
#include "Render/RenderStructs.hpp"

namespace Neo
{
    class IRenderContext
    {
    public:
        virtual ~IRenderContext() = default;
        FrameData& GetFrameData() { return mFrameDatas.at(mFrameIndex); }

        [[nodiscard]] virtual ResourceHandle GetResourceHandle(RenderTargetHandle handle) = 0;
        [[nodiscard]] virtual ResourceHandle GetResourceHandle(DepthStencilHandle handle) = 0;
        [[nodiscard]] virtual ResourceHandle GetResourceHandle(TextureHandle handle) = 0;
        [[nodiscard]] virtual ResourceHandle GetResourceHandle(BufferHandle handle) = 0;

        [[nodiscard]] virtual void* GetDevice() const = 0;
        [[nodiscard]] virtual void* GetGraphicsCommandQueue() const = 0;
        [[nodiscard]] virtual void* GetGraphicsCommandList() = 0;
        [[nodiscard]] virtual void* GetCBVUAVSRVAllocator() = 0;
        [[nodiscard]] virtual void* GetRenderDescriptor(RenderTargetHandle renderTargetHandle) = 0;
        [[nodiscard]] virtual void* GetTextureDescriptor(RenderTargetHandle renderTargetHandle) = 0;

        [[nodiscard]] virtual RenderTargetHandle CreateRenderTarget(RenderTargetCreateInfo createInfo,
                                                                    std::string_view debugName) = 0;
        [[nodiscard]] virtual DepthStencilHandle CreateDepthStencil(DepthStencilCreateInfo createInfo,
                                                                    std::string_view debugName) = 0;
        [[nodiscard]] virtual CommandHandle CreateCommand(QueueType queueType, std::string_view debugName) = 0;
        [[nodiscard]] virtual BufferHandle CreateBuffer(const BufferCreateInfo& createInfo,
                                                        std::string_view debugName) = 0;
        [[nodiscard]] virtual ShaderHandle CreateShader(const GraphicsShaderCreateInfo& createInfo,
                                                        std::string_view debugName) = 0;
        [[nodiscard]] virtual ShaderHandle CreateShader(const ComputeShaderCreateInfo& createInfo,
                                                        std::string_view debugName) = 0;

        virtual void DestroyRenderTarget(RenderTargetHandle renderTargetHandle) = 0;
        virtual void DestroyDepthStencil(DepthStencilHandle depthStencilHandle) = 0;
        virtual void DestroyBuffer(BufferHandle bufferHandle) = 0;

        [[nodiscard]] virtual void* MapBuffer(BufferHandle buffer) = 0;
        virtual void UnmapBuffer(BufferHandle buffer) = 0;
        virtual u32 GetGPUAddress(TextureHandle textureHandle) = 0;
        virtual u32 GetGPUAddress(BufferHandle textureHandle) = 0;

        virtual void OneTimeSubmit(const std::vector<CommandHandle>& commandHandle, QueueType queueType) = 0;
        virtual void Submit(const std::vector<CommandHandle>& commandHandle, QueueType queueType) = 0;
        virtual void Present() = 0;
        virtual void WaitForGPU() = 0;
        virtual void WaitForFrame() = 0;
        virtual void Resize() = 0;
        virtual void BeginCommand(CommandHandle commandHandle) = 0;
        virtual void SetupGraphicsCommand(CommandHandle commandHandle) = 0;
        virtual void EndCommand(CommandHandle commandHandle) = 0;
        virtual void BeginRenderPass(CommandHandle commandHandle, const RenderPassInfo& renderPassInfo) = 0;
        virtual void EndRenderPass(CommandHandle commandHandle) = 0;
        virtual void BindShader(CommandHandle commandHandle, ShaderHandle shaderHandle) = 0;
        virtual void SetViewport(CommandHandle commandHandle, const Viewport& viewport) = 0;
        virtual void SetScissor(CommandHandle commandHandle, const Scissor& scissor) = 0;
        virtual void SetPrimitiveTopology(CommandHandle commandHandle, PrimitiveTopology topology) = 0;
        virtual void
        Draw(CommandHandle commandHandle, u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 firstInstance) = 0;
        virtual void DrawIndexed(CommandHandle commandHandle,
                                 u32 indexCount,
                                 u32 instanceCount,
                                 u32 firstIndex,
                                 int vertexOffset,
                                 u32 firstInstance) = 0;
        virtual void BlitToSwapchain(CommandHandle commandHandle, RenderTargetHandle renderTargetHandle) = 0;
        virtual void PushConstant(CommandHandle commandHandle, u32 count, const void* data) = 0;
        virtual void CopyBuffer(CommandHandle commandHandle,
                                BufferHandle srcBuffer,
                                BufferHandle dstBuffer,
                                u64 srcOffset,
                                u64 dstOffset,
                                u64 size) = 0;

    protected:
        std::array<FrameData, kFrameCount> mFrameDatas = {};
        u32 mFrameIndex = 0;
    };
} // namespace FS
