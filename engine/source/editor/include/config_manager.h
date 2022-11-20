#ifndef _CONFIG_MANAGER_H__
#define _CONFIG_MANAGER_H__

#include <filesystem>

class ConfigManager {
public:
    void initialize(const std::filesystem::path& config_file_path);

    const std::filesystem::path& getRootFolder() const;
    const std::filesystem::path& getTexturePath() const;
    const std::filesystem::path& getShaderPath() const;

private:
    std::filesystem::path root_folder_;
    std::filesystem::path texture_path_;
    std::filesystem::path shader_path_;
};

#endif // _CONFIG_MANAGER_H__