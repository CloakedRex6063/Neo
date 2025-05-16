#pragma once


namespace Neo
{
    namespace JsonSerializer
    {
        template <typename T>
        bool Serialize(T& value, std::string_view path)
        {
            const auto errContext = glz::write_file_json<glz::opts{.indentation_width = 4}>(value, path, std::string{});
            if (errContext.ec != glz::error_code::none)
            {
                Log::Error("Failed to serialize: {}", magic_enum::enum_name(errContext.ec));
                return false;
            }
            return true;
        }

        template <typename T>
        bool Deserialize(T& value, std::string_view path)
        {
            const auto errContext = glz::read_file_json(value, path, std::string{});
            if (errContext.ec != glz::error_code::none)
            {
                Log::Error("Failed to deserialize: {}", magic_enum::enum_name(errContext.ec));
                return false;
            }
            return true;
        }
    }

    namespace BinarySerializer
    {
        template <typename T>
        bool Serialize(T& value, std::string_view path)
        {
            const auto errContext = glz::write_file_beve(value, path, std::string{});
            if (errContext.ec != glz::error_code::none)
            {
                Log::Error("Failed to serialize: {}", magic_enum::enum_name(errContext.ec));
                return false;
            }
            return true;
        }

        template <typename T>
        bool Deserialize(T& value, std::string_view path)
        {
            const auto errContext = glz::read_file_beve(value, path, std::string{});
            if (errContext.ec != glz::error_code::none)
            {
                Log::Error("Failed to deserialize: {}", magic_enum::enum_name(errContext.ec));
                return false;
            }
            return true;
        }
    }
}
