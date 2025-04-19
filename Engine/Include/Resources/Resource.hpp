#pragma once
#include "Tools/Tools.hpp"

namespace Neo
{
    struct IResource
    {
        
    };

    struct Primitive
    {
        int VertexOffset;
        int IndexOffset;
        int IndexCount;
        int MaterialIndex;
    };
    
    struct StaticMesh : IResource
    {
        std::vector<Primitive> Primitives;
        std::vector<u32> Indices;
        std::vector<u32> Vertices;
    };

    struct Material : IResource
    {
        UUID BaseColorTexture;
        UUID NormalTexture;
        UUID MetallicRoughnessTexture;
        UUID OcclusionTexture;
        UUID EmissiveTexture;

        glm::vec4 BaseColorFactor;
        glm::vec3 EmissiveFactor;
        float NormalFactor;
        float OcclusionFactor;
        float MetallicFactor;
        float RoughnessFactor;
    };

    enum class Wrap
    {
        eRepeat,
        eClamp,
        eMirror,
    };
    
    struct Texture : IResource
    {
        Wrap WrapX;
        Wrap WrapY;
    };
}
