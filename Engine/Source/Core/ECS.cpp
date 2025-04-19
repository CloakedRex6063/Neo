#include "Core/ECS.hpp"
#include "Generated.hpp"


namespace
{

}

namespace Neo
{
    ECS::ECS()
    {
        using namespace entt::literals;
        entt::meta_factory<std::string>().type("string"_hs);

        entt::meta_factory<u8>().type("u8"_hs);
        entt::meta_factory<u16>().type("u16"_hs);
        entt::meta_factory<u32>().type("u32"_hs);
        entt::meta_factory<u64>().type("u64"_hs);
        
        entt::meta_factory<int>().type("int"_hs);
        entt::meta_factory<i64>().type("i64"_hs);
        
        entt::meta_factory<float>().type("float"_hs);
        entt::meta_factory<double>().type("double"_hs);
        entt::meta_factory<bool>().type("bool"_hs);
        
        entt::meta_factory<glm::vec2>().type("glm::vec2"_hs);
        entt::meta_factory<glm::vec3>().type("vec3"_hs);
        entt::meta_factory<glm::vec4>().type("vec4"_hs);

        entt::meta_factory<glm::ivec2>().type("ivec2"_hs);
        entt::meta_factory<glm::ivec3>().type("ivec3"_hs);
        entt::meta_factory<glm::ivec4>().type("ivec4"_hs);

        entt::meta_factory<glm::uvec2>().type("uvec2"_hs);
        entt::meta_factory<glm::uvec3>().type("uvec3"_hs);
        entt::meta_factory<glm::uvec4>().type("uvec4"_hs);
        
        RegisterMeta();
    }
} // Neo
