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

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "directx/d3d12.h"
#include "directx/d3dx12.h"
#include "dxgi1_6.h"
#include "dxcapi.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "spdlog/fmt/fmt.h"
#include "entt/entt.hpp"

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

#include "Tools/Log.hpp"
#include "Tools/Warnings.hpp"