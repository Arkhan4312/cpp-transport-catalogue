#include "json_reader.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
namespace transport {

using namespace json;

JsonReader::JsonReader(TransportCatalogue& catalogue, const RequestHandler& handler)
    : catalogue_(catalogue),
      handler_(handler),
      data_loader_(std::make_unique<JsonDataLoader>(catalogue)),
      query_processor_(std::make_unique<QueryProcessor>(handler)) {
}

void JsonReader::LoadData(std::istream& input) {
    doc_ = Load(input);
    const Dict& root = doc_->GetRoot().AsMap();

    const Array& base_requests = root.at("base_requests").AsArray();
    
    data_loader_->LoadBaseRequest(base_requests);

    if (root.count("render_settings")) {
        render_settings_ = data_loader_->ParseRenderSettings(root.at("render_settings").AsMap());
    } else {
        throw std::logic_error("No render_settings");
    }
}

void JsonReader::ProcessRequests(std::ostream& output) const {
    if (!doc_ || !render_settings_) {
        throw std::logic_error("Data not loaded or render settings missing");
    }

    const auto& root = doc_->GetRoot().AsMap();
    const auto& stat_request = root.at("stat_requests").AsArray();

    auto answers = query_processor_->ProcessRequests(stat_request, *render_settings_);
    Document out_doc(std::move(answers));
    Print(out_doc, output);
}

RenderSettings JsonReader::GetRenderSettings() const {
    if (!render_settings_) {
        throw std::logic_error("Render settings not loaded");
    }
    return *render_settings_;
}

void JsonReader::MapToFile(const std::string& filename) const {
    if (!render_settings_) {
        throw std::logic_error("Render settings not loaded");
    }
    std::string svg = handler_.RenderMap(*render_settings_);
    SaveMapToFile(svg, filename);
}

}  // namespace transport