#pragma once

namespace Neo
{
    class FileIO final
    {
    public:
        FileIO();
        
        enum class Directory
        {
            eAssets,
            eShaders,
            eSaveFiles,
            eNone
        };
        /// <summary>
        /// Read a text file into a string. The string is empty if the file was not found.
        /// </summary>
        [[nodiscard]] std::string ReadTextFile(Directory type, std::string_view path) const;

        /// <summary>
        /// Write a string to a text file. The file is created if it does not exist.
        /// Returns true if the file was written successfully.
        /// </summary>
        [[nodiscard]] bool WriteTextFile(Directory type, std::string_view path, std::string_view content) const;

        /// <summary>
        /// Read a binary file into a vector of chars. The vector is empty if the file was not found.
        /// </summary>
        [[nodiscard]] std::vector<char> ReadBinaryFile(Directory type, std::string_view path) const;

        /// <summary>
        /// Write a string to a binary file. The file is created if it does not exist.
        /// Returns true if the file was written successfully.
        /// </summary>
        [[nodiscard]] bool
        WriteBinaryFile(Directory type, std::string_view path, const std::vector<char>& content) const;

        /// <summary>
        /// Get the full path of a file.
        /// </summary>
        [[nodiscard]] std::string GetPath(Directory type, std::string_view path) const;

        /// <summary>
        /// Check if a file exists.
        /// </summary>
        [[nodiscard]] bool Exists(Directory type, std::string_view path) const;

        /// <summary>
        /// Check the last time a file was modified. Only used on desktop platforms.
        /// </summary>
        [[nodiscard]] uint64_t LastModified(Directory type, std::string_view path) const;

    private:
        std::map<Directory, std::string> mPaths;
    };
} // namespace FS