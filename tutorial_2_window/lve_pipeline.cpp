#include "lve_pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

namespace lve {
    std::vector<char> LvePipeline::ReadFile(const std::string& file_name) {

        // ios::ate = bit flag to seek end immediately after opening. easier to get size.
        // ios::binary = bit flag to read as binary. prevents unwanted text/string transform.
        std::ifstream file{file_name, std::ios::ate | std::ios::binary};
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file" + file_name);
        }
        // [tellg] gets position of stream/file/char. Since we start from the end due to ios::ate,
        // we can use this to get the numel of chars, which is used for initializing buffer.
        size_t file_size = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(file_size);

        // [seekg] sets position of stream/file to an index. which is 0/start for our use case.
        // this is used to start reading the data in the file.
        file.seekg(0);
        // [read] stores "file_size" amount of data from current stream position into pointer
        // given by buffer.data().
        file.read(buffer.data(), file_size);
        file.close();
        return buffer;
    }

    void LvePipeline::CreateGraphicPipeline(const std::string& vert_file_path, const std::string& frag_file_path) {
        auto vertCode = ReadFile(vert_file_path);
        auto fragCode = ReadFile(frag_file_path);

        std::cout << "vert code size:" << vertCode.size() << "\n";
        std::cout << "frag code size:" << fragCode.size() << "\n";
    }

    LvePipeline::LvePipeline(const std::string& vert_file_path, const std::string& frag_file_path) {
        CreateGraphicPipeline(vert_file_path, frag_file_path);
    }

} // namespace lve