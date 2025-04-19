#pragma once

namespace Neo
{
    struct Name
    {
        [[Serialize]]
        std::string EntityName;
    };

    struct Transform
    {
        [[Serialize]]
        glm::vec3 Position;
        [[Serialize]]
        glm::vec3 Rotation;
        [[Serialize]]
        glm::vec3 Scale;
    };

    struct Hierarchy
    {
        [[Serialize]]
        Entity Parent = NullEntity;
        [[Serialize]]
        std::vector<Entity> Children;
    };
}
