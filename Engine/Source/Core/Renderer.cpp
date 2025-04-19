#include "Core/Renderer.hpp"
#include "Core/Device.hpp"
#include "Core/Engine.hpp"
#include "Render/DX12/RenderContextDX12.hpp"
#include "Render/RenderComponents.hpp"

namespace Neo
{
    Renderer::Renderer()
    {
        RenderContextCreateInfo createArgs{
            .GpuPreference = DevicePreference::eHighPerformance,
        };
        mContext = std::make_unique<RenderContextDX12>(createArgs);

        constexpr BufferCreateInfo uploadCreateInfo{
            .FirstElement = 0, .NumElements = 3, .Stride = sizeof(Vertex), .Type = BufferType::eStaging
        };
        const auto uploadBuffer = mContext->CreateBuffer(uploadCreateInfo, "Vertex Upload Buffer");

        mTransferCommand = mContext->CreateCommand(QueueType::eTransfer, "Transfer");

        constexpr std::array vertexData{
            Vertex{.Position = glm::vec3(-0.5f, -0.5f, 0.0f)},
            Vertex{.Position = glm::vec3(0.5f, -0.5f, 0.0f)},
            Vertex{.Position = glm::vec3(0.0f, 0.5f, 0.0f)},
        };

        auto* const mappedPtr = mContext->MapBuffer(uploadBuffer);
        std::memcpy(mappedPtr, vertexData.data(), sizeof(Vertex) * 3);
        mContext->UnmapBuffer(uploadBuffer);

        constexpr BufferCreateInfo vertexCreateInfo{
            .FirstElement = 0, .NumElements = 3, .Stride = sizeof(Vertex), .Type = BufferType::eStorage
        };
        mVertexBuffer = mContext->CreateBuffer(vertexCreateInfo, "Vertex Buffer");

        mContext->BeginCommand(mTransferCommand);
        mContext->CopyBuffer(mTransferCommand, uploadBuffer, mVertexBuffer, 0, 0, sizeof(Vertex) * 3);
        mContext->EndCommand(mTransferCommand);
        mContext->OneTimeSubmit({mTransferCommand}, QueueType::eTransfer);

        const GraphicsShaderCreateInfo shaderCreateInfo{
            .VertexCode = Engine.FileIO().ReadBinaryFile(FileIO::Directory::eShaders, "GeomVS.cso"),
            .FragmentCode = Engine.FileIO().ReadBinaryFile(FileIO::Directory::eShaders, "GeomPS.cso"),
            .PrimitiveTopology = PrimitiveTopology::eTriangle,
            .RenderTargetFormats = {Format::eB8G8R8A8_UNORM},
            .NumRenderTargets = 1,
            .DepthStencilFormat = Format::eUnknown,
        };
        mTriangleShader = mContext->CreateShader(shaderCreateInfo, "Triangle Shader");

        const auto size = Engine.Device().GetWindowSize();
        const RenderTargetCreateInfo renderTargetCreateInfo{
            .Size = {size.x, size.y},
            .Format = Format::eB8G8R8A8_UNORM,
            .ViewType = ViewType::eTexture2D,
        };
        mRenderTarget = mContext->CreateRenderTarget(renderTargetCreateInfo, "Render Target");


        RenderPass renderPass
        {
            .Execute = []
            {
                const auto& context = Engine.Renderer().GetRenderContext();
                const auto& command = context->GetFrameData().CommandHandle;
                const RenderPassInfo renderPassInfo{
                    .RenderTargets = {Engine.Renderer().mRenderTarget},
                    .ClearColor = glm::vec4(0.392f, 0.584f, 0.929f, 1.0f),
                };
                context->BeginRenderPass(command, renderPassInfo);
                context->BindShader(command, Engine.Renderer().mTriangleShader);
                context->SetPrimitiveTopology(command, PrimitiveTopology::eTriangle);
                const Viewport viewport{.Dimensions = Engine.Device().GetWindowSize()};
                context->SetViewport(command, viewport);
                const Scissor scissor{.Max = Engine.Device().GetWindowSize()};
                context->SetScissor(command, scissor);

                const struct PushConstant
                {
                    u32 index = 0;
                } pc{
                        .index = context->GetGPUAddress(Engine.Renderer().mVertexBuffer),
                    };
                context->PushConstant(command, 1, &pc);
                context->Draw(command, 3, 1, 0, 0);
                context->EndRenderPass(command);
            }
        };
        AddRenderPass(std::move(renderPass));
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::Update(float)
    {
        const auto& [command, renderTarget, fenceValue] = mContext->GetFrameData();
        if (Engine.Device().IsWindowResized())
        {
            GetRenderContext()->WaitForGPU();
            mContext->Resize();
            mContext->DestroyRenderTarget(mRenderTarget);
            const auto size = Engine.Device().GetWindowSize();
            const RenderTargetCreateInfo renderTargetCreateInfo{
                .Size = {size.x, size.y},
                .Format = Format::eB8G8R8A8_UNORM,
                .ViewType = ViewType::eTexture2D,
            };
            mRenderTarget = mContext->CreateRenderTarget(renderTargetCreateInfo, "Render Target");
        }
        else
        {
            mContext->BeginCommand(command);
            mContext->SetupGraphicsCommand(command);
            
            for (auto& renderPass : mRenderPasses)
            {
                if (renderPass.Execute)
                {
                    renderPass.Execute();
                }
            }

            mContext->EndCommand(command);
            mContext->Submit({command}, QueueType::eGraphics);
            mContext->Present();
        }
    }

    void Renderer::AddRenderPass(RenderPass&& renderPass)
    {
        mRenderPasses.emplace_back(std::move(renderPass));
    }
} // namespace FS
