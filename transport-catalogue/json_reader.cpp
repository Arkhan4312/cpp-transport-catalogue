#include "json_reader.h"

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

    auto base_it = root.find("base_requests");
    if (base_it != root.end()) {
        const auto& base_requests = base_it->second.AsArray();
        data_loader_->LoadBaseRequest(base_requests);
    }

    auto stat_it = root.find("stat_requests");
    if (stat_it != root.end()) {
        stat_requests_ = stat_it->second.AsArray();
    }

    auto render_it = root.find("render_settings");
    if (render_it != root.end()) {
        render_settings_ = data_loader_->ParseRenderSettings(render_it->second.AsMap());
    }
}

void JsonReader::ProcessRequests(std::ostream& output) const {
    if (!stat_requests_.has_value()) {
        json::Print(json::Document{json::Array{}}, output);
        return;
    }
    auto answers = query_processor_->ProcessRequests(stat_requests_.value(), render_settings_);
    json::Document doc(std::move(answers));
    json::Print(doc, output);
}

}  // namespace transport