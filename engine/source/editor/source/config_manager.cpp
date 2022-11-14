#include "editor/include/config_manager.h"

#include <filesystem>
#include <fstream>
#include <string>

void ConfigManager::initialize(const std::filesystem::path& config_file_path) {
    std::ifstream config_file(config_file_path);
    std::string   config_line;
    while (std::getline(config_file, config_line))
    {
        size_t seperate_pos = config_line.find_first_of('=');
        if (seperate_pos > 0 && seperate_pos < (config_line.length() - 1))
        {
            std::string name  = config_line.substr(0, seperate_pos);
            std::string value = config_line.substr(seperate_pos + 1, config_line.length() - seperate_pos - 1);
            if (name == "BinaryRootFolder") {
                root_folder_ = config_file_path.parent_path() / value;
            } else if (name == "TexturePath") {
                texture_path_ = root_folder_ / value;
            } else if (name == "ShaderPath") {
                shader_path_ = root_folder_ / value;
            }
        }
    }
}

const std::filesystem::path& ConfigManager::getRootFolder() const { return root_folder_;}

const std::filesystem::path& ConfigManager::getTexturePath() const { return texture_path_;}

const std::filesystem::path& ConfigManager::getShaderPath() const { return shader_path_;}