#pragma once
#include "Core/Resources.hpp"
#include "Resources/Resource.hpp"

namespace Neo
{
    class Resources
    {
    public:
        Resources();
        ~Resources();

        void CleanupResources();

    private:
        std::shared_ptr<IResource> mResources;
    };
}
