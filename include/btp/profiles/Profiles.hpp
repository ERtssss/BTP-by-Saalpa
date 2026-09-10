#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace btp::profiles {
class Profiles {
public:
    static Profiles& get();
    void setDirectory(std::filesystem::path path);
    std::vector<std::string> list() const;
    bool load(const std::string& name);
    bool save(const std::string& name) const;
    bool remove(const std::string& name) const;
private:
    std::filesystem::path mDirectory;
};
}
