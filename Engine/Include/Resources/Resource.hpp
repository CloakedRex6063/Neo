#pragma once
#include "Render/RenderComponents.hpp"
#include "Render/RenderEnums.hpp"

namespace Neo
{
    struct IResource
    {
        
    };

    enum class Wrap
    {
        eRepeat,
        eClamp,
        eMirror,
    };

    struct TextureDesc 
    {
        std::string Name;
        AssetID ID;
        Wrap WrapX;
        Wrap WrapY;
        uint32_t Width;
        uint32_t Height;
        uint16_t ArraySize = 1;
        uint16_t MipLevels = 1;
        Format Format = Format::eR8G8B8A8_UNORM;
        std::vector<uint8_t> Pixels;
    };
    
    struct Texture : IResource
    {
        Wrap WrapX;
        Wrap WrapY;
        uint32_t Width;
        uint32_t Height;
        uint16_t ArraySize = 1;
        uint16_t MipLevels = 1;
        Format Format = Format::eR8G8B8A8_UNORM;
        std::vector<uint8_t> Pixels;
    };

    struct MaterialDesc
    {
        std::string Name;
        AssetID ID;
        Opt<AssetID> BaseColorTextureID;
        Opt<AssetID> NormalTextureID;
        Opt<AssetID> MetallicRoughnessTextureID;
        Opt<AssetID> OcclusionTextureID;
        Opt<AssetID> EmissiveTextureID;

        glm::vec4 BaseColorFactor;
        glm::vec3 EmissiveFactor;
        float NormalFactor;
        float OcclusionFactor;
        float MetallicFactor;
        float RoughnessFactor;
    };
    
    struct Material : IResource
    {
        AssetID ID;
        Ref<Texture> BaseColorTexture;
        Ref<Texture> NormalTexture;
        Ref<Texture> MetallicRoughnessTexture;
        Ref<Texture> OcclusionTexture;
        Ref<Texture> EmissiveTexture;

        glm::vec4 BaseColorFactor;
        glm::vec3 EmissiveFactor;
        float NormalFactor;
        float OcclusionFactor;
        float MetallicFactor;
        float RoughnessFactor;
    };

    struct Primitive
    {
        std::vector<u32> Indices;
        std::vector<Vertex> Vertices;
        Ref<Material> Material;
    };
    
    struct Mesh : IResource
    {
        std::vector<Primitive> Primitives;
    };

    struct PrimitiveDesc
    {
        std::vector<u32> Indices;
        std::vector<Vertex> Vertices;
        Opt<AssetID> MaterialID;
    };
    
    struct MeshDesc
    {
        std::string Name;
        AssetID ID;
        std::vector<PrimitiveDesc> Primitives;
    };

    struct NodeDesc
    {
        std::vector<AssetID> Meshes;
        std::vector<uint64_t> Children;
    };

    struct Model
    {
        std::vector<NodeDesc> Nodes;
        std::vector<uint64_t> RootNodes;
    };
}
