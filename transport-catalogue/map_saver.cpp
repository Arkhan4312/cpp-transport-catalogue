#include "map_saver.h"

#include <fstream>
#include <stdexcept>
namespace transport {
void SaveMapToFile(const std::string& svg_content, const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        throw std::logic_error("Cannot open file");
    }
    out << svg_content;
}
}  // namespace transport
