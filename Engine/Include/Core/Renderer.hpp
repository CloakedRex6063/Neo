#pragma once
#include "Render/RenderContext.hpp"

namespace Neo
{
    struct RenderPass
    {
        std::function<void()> Execute;
    };
    class Renderer final
    {
    public:
        Renderer();
        ~Renderer();
        void Update(float);
        [[nodiscard]] std::unique_ptr<IRenderContext>& GetRenderContext() { return mContext; }
        [[nodiscard]] RenderTargetHandle GetRenderTarget() const { return mRenderTarget; }

        void AddRenderPass(RenderPass&& renderPass);

    private:
        std::unique_ptr<IRenderContext> mContext;
        BufferHandle mVertexBuffer = BufferHandle::eNull;
        ShaderHandle mTriangleShader = ShaderHandle::eNull;
        RenderTargetHandle mRenderTarget = RenderTargetHandle::eNull;
        CommandHandle mTransferCommand = CommandHandle::eNull;

        std::vector<RenderPass> mRenderPasses;
    };
} // namespace FS
