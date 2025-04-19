#include "Core/FileIO.hpp"
#include "Tools/Log.hpp"

namespace Neo
{
    FileIO::FileIO()
    {
        mPaths[Directory::eAssets] = "Assets/";
        mPaths[Directory::eSaveFiles] = "Saves/";
        mPaths[Directory::eShaders] = "Shaders/";
        mPaths[Directory::eNone] = "";
    }

    std::string FileIO::ReadTextFile(const Directory type, const std::string_view path) const
    {
        const auto fullPath = GetPath(type, path);
        std::ifstream file(fullPath, std::ios::in);
        if (!file.is_open())
        {
            Log::Error("File {} with full path {} was not found!", path, fullPath);
            return {};
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool FileIO::WriteTextFile(const Directory type, const std::string_view path, const std::string_view content) const
    {
        const auto fullPath = GetPath(type, path);
        std::ofstream file(fullPath);
        if (!file.is_open())
        {
            Log::Error("File {} with full path {} was not found!", path, fullPath);
            return false;
        }
        file << content;
        file.close();
        return true;
    }

    std::vector<char> FileIO::ReadBinaryFile(const Directory type, const std::string_view path) const
    {
        const auto fullPath = GetPath(type, path);
        std::ifstream file(fullPath, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            Log::Error("File {} with full path {} was not found!", path, fullPath);
            return {};
        }
        const int64_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        if (file.read(buffer.data(), size))
            return buffer;
        assert(false);
        return {};
    }

    bool FileIO::WriteBinaryFile(const Directory type, const std::string_view path, const std::vector<char>& content) const
    {
        const auto fullPath = GetPath(type, path);
        std::ofstream file(fullPath, std::ios::binary);
        if (!file.is_open())
        {
            Log::Error("File {} with full path {} was not found!", path, fullPath);
            return false;
        }
        file.write(content.data(), static_cast<int64_t>(content.size()));
        file.close();
        return true;
    }

    std::string FileIO::GetPath(const Directory type, const std::string_view path) const
    {
        const std::filesystem::path dir = mPaths.at(type);
        return std::filesystem::absolute(mPaths.at(type) + path.data()).string();
    }

    bool FileIO::Exists(const Directory type, const std::string_view path) const
    {
        const auto fullPath = GetPath(type, path);
        return !std::filesystem::exists(fullPath);
    }

    uint64_t FileIO::LastModified(const Directory type, const std::string_view path) const
    {
        const auto fullPath = GetPath(type, path);
        const std::filesystem::file_time_type fTime = std::filesystem::last_write_time(fullPath);
        return static_cast<uint64_t>(fTime.time_since_epoch().count());
    }
} // namespace FS
