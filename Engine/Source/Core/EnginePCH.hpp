#pragma once

#include "map"
#include "chrono"
#include "memory"
#include "vector"
#include "string"
#include "ranges"
#include "algorithm"
#include "string_view"
#include "unordered_map"
#include "unordered_set"
#include "filesystem"
#include "functional"
#include "fstream"
#include "expected"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOCRYPT       
#define NOCOM       
#define NORPC        
#define NOIME        
#include "Windows.h"
#include "directx/d3d12.h"
#include "directx/d3dx12.h"
#include "dxgi1_6.h"
#include "dxcapi.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "spdlog/fmt/fmt.h"
#include "entt/entt.hpp"
#include "uuid.h"
#include "glaze/glaze.hpp"
#include "magic_enum/magic_enum.hpp"

using World = entt::registry;
using Entity = entt::entity;
inline constexpr auto NullEntity = entt::null; 

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

namespace Neo
{
    template<typename T>
using Ref = std::shared_ptr<T>;

    template<typename T>
    using Scoped = std::unique_ptr<T>;

    template<typename T>
    using Opt = std::optional<T>;

    template<typename T, typename Error>
    using Exp = std::expected<T, Error>;

    using AssetID = uuids::uuid;
}

#include "Tools/Log.hpp"
#include "Tools/Warnings.hpp"

namespace glz {

    template <>
    struct meta<glm::vec4> {
        static constexpr auto value = object(
            "x", &glm::vec4::x,
            "y", &glm::vec4::y,
            "z", &glm::vec4::z,
            "w", &glm::vec4::w
        );
    };

    template <>
    struct meta<glm::vec3> {
        static constexpr auto value = object(
            "x", &glm::vec3::x,
            "y", &glm::vec3::y,
            "z", &glm::vec3::z
        );
    };

    template <>
    struct meta<Neo::AssetID>
    {
        static constexpr auto read_x = [](Neo::AssetID& s, const std::string& input) { s = uuids::uuid{input.begin(), input.end()}; };
        static constexpr auto write_x = [](Neo::AssetID& s) -> auto { return s.as_bytes(); };
        static constexpr auto value = glz::object("str", glz::custom<read_x, write_x>);
    };
}


