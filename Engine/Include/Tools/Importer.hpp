#pragma once
#include "string_view"
#include "fastgltf/core.hpp"
#include "Resources/Resource.hpp"

namespace Neo
{
    class Importer
    {
    public:
        enum class Error
        {
            eFileNotFound,
            eInvalidAsset,
            eNoIndices,
            eNoPositions,
            eNoTexCoords,
            eNoNormals,
            eNoImage,
        };

        static Exp<void, Error> ImportGLTF(std::string_view inPath, std::string_view outPath);

    private:
        static Exp<std::vector<MeshDesc>, Error> ImportMeshes(const fastgltf::Asset& asset, std::span<const MaterialDesc> materials, std::string_view outPath);
        static Exp<std::vector<MaterialDesc>, Error> ImportMaterials(const fastgltf::Asset& asset, std::span<const TextureDesc> textures, std::string_view outPath);
        static Exp<std::vector<TextureDesc>, Error> ImportTextures(const fastgltf::Asset& asset, std::string_view inPath, std::string_view outPath);
        static Exp<Model, Error> ImportModel(const fastgltf::Asset& asset);
    };
}
