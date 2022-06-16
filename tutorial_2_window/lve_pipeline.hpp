#pragma once

#include <string>
#include <vector>

namespace lve {
class LvePipeline {
    public:
    LvePipeline(const std::string& vert_file_path, const std::string& frag_file_path);

    private:
    static std::vector<char> ReadFile(const std::string& file_name);
    void CreateGraphicPipeline(const std::string& vert_file_path, const std::string& frag_file_path);
};
} // namespace lve