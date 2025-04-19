#pragma once
#include "Core/Components.hpp"

namespace Neo
{
    class ECS
    {
    public:
        ECS();

        Entity CreateEntity(const std::string_view name)
        {
            const auto entity = mWorld.create();
            mWorld.emplace<Name>(entity, std::string(name));
            mWorld.emplace<Transform>(entity);
            mWorld.emplace<Hierarchy>(entity);
            return entity;
        }

        Entity CreateNamelessEntity()
        {
            const auto entity = mWorld.create();
            mWorld.emplace<Transform>(entity);
            mWorld.emplace<Hierarchy>(entity);
            return entity;
        }

        std::string_view GetName(const Entity entity)
        {
            return mWorld.get<Name>(entity).EntityName;
        }

        void SetName(const Entity entity, const std::string_view name)
        {
            mWorld.get<Name>(entity).EntityName = std::string(name);
        }

        void AddChild(const Entity parent, const Entity child)
        {
            mWorld.patch<Hierarchy>(parent, [&](Hierarchy& hierarchy)
            {
                hierarchy.Children.emplace_back(child);
            });
            mWorld.patch<Hierarchy>(child, [&](Hierarchy& hierarchy)
            {
                hierarchy.Parent = parent;
            });
        }

        Entity GetParent(const Entity entity)
        {
            return mWorld.get<Hierarchy>(entity).Parent;
        }

        std::span<Entity> GetChildren(const Entity entity)
        {
            return mWorld.get<Hierarchy>(entity).Children;
        }

        template <typename... T>
        decltype(auto) View() const
        {
            return mWorld.view<T...>();
        }

        template <typename T, typename... Args>
        void Add(const Entity entity, Args&&... args)
        {
            static_assert(!std::is_same_v<T, Hierarchy>, "Hierarchy cannot be added manually");
            static_assert(!std::is_same_v<T, Name>, "Name cannot be added manually");
            static_assert(!std::is_same_v<T, Transform>, "Transform cannot be added manually");
            mWorld.emplace<T>(entity, std::forward<Args>(args)...);
        }

        template <typename... T>
        bool Has(const Entity entity) const
        {
            return mWorld.all_of<T...>(entity);
        }

        template <typename... T>
        decltype(auto) Get(const Entity entity) const
        {
            static_assert((... && !std::is_same_v<T, Hierarchy>), "Use the parenting functions from ECS instead");
            static_assert((... && !std::is_same_v<T, Name>), "Use ECS().GetName() instead");
            return mWorld.get<const T...>(entity);
        }

        template <typename T, typename... Func>
        void Modify(const Entity entity, Func&&... func)
        {
            static_assert(!std::is_same_v<T, Hierarchy>,
                          "Hierarchy cannot be modified this way, use the functions from ECS instead");
            static_assert(!std::is_same_v<T, Name>, "Name cannot be modified this way, use ECS().SetName() instead");
            mWorld.patch<T>(entity, std::forward<Func>(func)...);
        }

        World& GetWorld() { return mWorld; }

    private:
        World mWorld;
    };
} // Neo
