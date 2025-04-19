#pragma once

namespace Neo
{
    struct ProjectInfo
    {
        std::string Name;
        std::string Version;
    };

    struct ProjectData
    {
        ProjectInfo Info;
        std::string Path;
    };
    class Project
    {
    public:
        void LoadProject(const std::filesystem::path& projectPath);
        void GenerateProject(const ProjectInfo& projectInfo, const std::filesystem::path& projectPath) const;

    private:
        ProjectData mProjectData;
    };
}