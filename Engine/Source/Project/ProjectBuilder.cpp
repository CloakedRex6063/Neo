#include "Project/ProjectBuilder.hpp"

#include "simdjson.h"
#include "nlohmann/json.hpp"

namespace Neo
{
    void Project::LoadProject(const std::filesystem::path& projectPath)
    {
        simdjson::dom::parser parser;
        const simdjson::dom::element doc = parser.load(projectPath.generic_string());
        
        mProjectData = ProjectData
        {
            .Info = {.Name = std::string(doc["Project"]["Name"]), .Version = std::string(doc["Project"]["Version"])},
            .Path = projectPath.string(),
        };
    }

    void Project::GenerateProject(const ProjectInfo& projectInfo, const std::filesystem::path& projectPath) const
    {
        nlohmann::json json;
        json["Project"]["Name"] = projectInfo.Name;
        json["Project"]["Version"] = projectInfo.Version;

        if (!std::filesystem::exists(projectPath))
        {
            std::filesystem::create_directory(projectPath);
        }

        std::string filePath = projectPath.string() + "/" + projectInfo.Name + ".proj";
        
        std::ofstream file(filePath);
        file << json.dump(4);
        file.close();
    }
}
