#include "Tools/Importer.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/glm_element_traits.hpp"
#include "stb_image.h"
#include "Core/FileIO.hpp"
#include "Tools/Serializer.hpp"

namespace
{
    fastgltf::Parser gParser;
}

Neo::Exp<void, Neo::Importer::Error> Neo::Importer::ImportGLTF(const std::string_view inPath, std::string_view outPath)
{
    auto expMappedFile = fastgltf::MappedGltfFile::FromPath(inPath);
    if (!expMappedFile)
    {
        Log::Error("Failed to load gltf file: {}", magic_enum::enum_name(expMappedFile.error()));
        return std::unexpected(Error::eFileNotFound);
    }

    const auto directory = std::filesystem::path(inPath).parent_path();

    auto expAsset = gParser.loadGltf(expMappedFile.get(), directory,
                                     fastgltf::Options::LoadExternalBuffers);

    if (!expAsset)
    {
        return std::unexpected(Error::eInvalidAsset);
    }

    const auto asset = std::move(expAsset.get());
    const auto textures = ImportTextures(asset, directory.string(), outPath);
    if (!textures) return std::unexpected(textures.error());
    const auto materials = ImportMaterials(asset, textures.value(), outPath);
    if (!materials) return std::unexpected(materials.error());
    const auto meshes = ImportMeshes(asset, materials.value(), outPath);
    if (!meshes) return std::unexpected(meshes.error());
    const auto model = ImportModel(asset);

    return {};
}

Neo::Exp<std::vector<Neo::MeshDesc>, Neo::Importer::Error> Neo::Importer::ImportMeshes(
    const fastgltf::Asset& asset, const std::span<const MaterialDesc> materials, const std::string_view outPath)
{
    std::vector<MeshDesc> meshes;
    for (const auto& mesh : asset.meshes)
    {
        MeshDesc m{};
        m.ID = GenerateUUID();
        m.Name = mesh.name;
        for (const auto& primitive : mesh.primitives)
        {
            if (!primitive.indicesAccessor) return std::unexpected(Error::eNoIndices);

            PrimitiveDesc p;
            const auto& indexAccessor = asset.accessors.at(primitive.indicesAccessor.value());
            p.Indices.resize(indexAccessor.count);

            switch (indexAccessor.componentType)
            {
            case fastgltf::ComponentType::UnsignedShort:
                {
                    std::vector<uint16_t> shortIndices(indexAccessor.count);
                    fastgltf::copyFromAccessor<uint16_t>(asset, indexAccessor, shortIndices.data());
                    std::ranges::copy(shortIndices, p.Indices.begin());
                    break;
                }
            case fastgltf::ComponentType::UnsignedInt:
                {
                    fastgltf::copyFromAccessor<uint32_t>(asset, indexAccessor, p.Indices.data());
                    break;
                }
            default:
                break;
            }

            if (!primitive.findAttribute("POSITION")) return std::unexpected(Error::eNoPositions);
            const auto& positionAccessor = asset.accessors.at(primitive.findAttribute("POSITION")->accessorIndex);
            p.Vertices.resize(positionAccessor.count);

            fastgltf::iterateAccessorWithIndex<glm::vec3>(asset,
                                                          positionAccessor,
                                                          [&](const glm::vec3& position, const size_t index)
                                                          {
                                                              p.Vertices[index].Position = position;
                                                          });

            if (primitive.findAttribute("NORMAL")->accessorIndex)
            {
                const auto& normalAccessor = asset.accessors[primitive.findAttribute("NORMAL")->accessorIndex];
                fastgltf::iterateAccessorWithIndex<glm::vec3>(asset,
                                                              normalAccessor,
                                                              [&](const glm::vec3& normal, const size_t index)
                                                              {
                                                                  p.Vertices[index].Normal = normal;
                                                              });
            }

            if (primitive.findAttribute("TEXCOORD_0")->accessorIndex)
            {
                const auto& uvAccessor = asset.accessors[primitive.findAttribute("TEXCOORD_0")->accessorIndex];
                fastgltf::iterateAccessorWithIndex<glm::vec2>(asset,
                                                              uvAccessor,
                                                              [&](const glm::vec2& uv, const size_t index)
                                                              {
                                                                  p.Vertices[index].UVx = uv.x;
                                                                  p.Vertices[index].UVy = uv.y;
                                                              });
            }

            if (primitive.materialIndex)
            {
                p.MaterialID = materials[primitive.materialIndex.value()].ID;
            }
            m.Primitives.emplace_back(std::move(p));
        }
        BinarySerializer::Serialize(m, std::string(outPath) + std::string(m.Name));
        meshes.emplace_back(std::move(m));
    }
    return meshes;
}

Neo::Exp<std::vector<Neo::MaterialDesc>, Neo::Importer::Error> Neo::Importer::ImportMaterials(
    const fastgltf::Asset& asset, const std::span<const TextureDesc> textures, const std::string_view outPath)
{
    std::vector<MaterialDesc> materials;
    for (const auto& material : asset.materials)
    {
        const auto& [baseColorFactor, metallicFactor, roughnessFactor, baseColorTexture, metallicRoughnessTexture] =
            material.pbrData;
        MaterialDesc m;
        m.Name = material.name;
        m.ID = GenerateUUID();
        m.BaseColorFactor = glm::make_vec4(baseColorFactor.data());
        if (baseColorTexture)
        {
            m.BaseColorTextureID = textures[baseColorTexture->textureIndex].ID;
        }
        m.MetallicFactor = metallicFactor;
        if (metallicRoughnessTexture)
        {
            m.MetallicRoughnessTextureID = textures[metallicRoughnessTexture->textureIndex].ID;
        }
        m.RoughnessFactor = roughnessFactor;
        m.EmissiveFactor = glm::make_vec3(material.emissiveFactor.data());
        if (material.emissiveTexture)
        {
            m.EmissiveTextureID = textures[material.emissiveTexture->textureIndex].ID;
        }

        if (material.normalTexture)
        {
            m.NormalFactor = material.normalTexture->scale;
            m.NormalTextureID = textures[material.normalTexture->textureIndex].ID;
        }

        if (material.occlusionTexture)
        {
            m.OcclusionFactor = material.occlusionTexture->strength;
            m.OcclusionTextureID = textures[material.occlusionTexture->textureIndex].ID;
        }
        BinarySerializer::Serialize(m, std::string(outPath) + '/' + std::string(m.Name));
        materials.emplace_back(std::move(m));
    }
    return materials;
}

Neo::Exp<std::vector<Neo::TextureDesc>, Neo::Importer::Error> Neo::Importer::ImportTextures(
    const fastgltf::Asset& asset, const std::string_view inPath, const std::string_view outPath)
{
    std::vector<TextureDesc> textures;
    for (const auto& [index, texture] : std::views::enumerate(asset.textures))
    {
        TextureDesc t{};
        t.Name = texture.name;
        if (t.Name == "")
        {
            t.Name = "Texture" + std::to_string(index);
        }
        t.ID = GenerateUUID();
        if (texture.imageIndex)
        {
            const auto& [data, name] = asset.images.at(texture.imageIndex.value());
            std::vector<char> rawData;
            if (std::holds_alternative<fastgltf::sources::URI>(data))
            {
                const auto path = std::get<fastgltf::sources::URI>(data).uri.string();
                rawData = FileIO::ReadBinaryFile(std::string(inPath) + '/' + std::string(path));
            }
            int channels = 4, width, height;
            auto* pixels = stbi_load_from_memory(reinterpret_cast<stbi_uc const*>(rawData.data()),
                                                  static_cast<int>(rawData.size()), &width, &height,
                                                  &channels, STBI_rgb_alpha);

            if (!pixels) return std::unexpected(Error::eNoImage);

            t.Width = static_cast<uint32_t>(width);
            t.Height = static_cast<uint32_t>(height);
            t.Pixels.assign(pixels, pixels + t.Width * t.Height * 4);
            stbi_image_free(pixels);
        }
        else if (texture.ddsImageIndex)
        {
            // TODO:
        }
        BinarySerializer::Serialize(t, std::string(outPath) + '/' + std::string(t.Name));
        textures.emplace_back(std::move(t));
    }
    return textures;
}

Neo::Exp<Neo::Model, Neo::Importer::Error> Neo::Importer::ImportModel(const fastgltf::Asset& asset)
{
    std::vector<NodeDesc> nodes(asset.nodes.size());
    std::ranges::transform(asset.nodes, nodes.begin(), [&](const fastgltf::Node& node)
    {
        std::vector<uint64_t> children(node.children.size());
        std::ranges::copy(node.children, children.begin());
        NodeDesc nodeDesc{
            .Meshes = {},
            .Children = std::move(children),
        };
        return nodeDesc;
    });

    Model m;
    m.Nodes = nodes;
    m.RootNodes.resize(nodes.size());
    std::ranges::transform(asset.scenes[0].nodeIndices, m.RootNodes.begin(), [&](const uint64_t& node)
    {
        return static_cast<uint64_t>(node);
    });

    return m;
}
