#include "json_reader.h"

#include "json_builder.h"

namespace transport {

using namespace json;

JsonReader::JsonReader(TransportCatalogue& catalogue, RequestHandler& handler)
    : handler_(handler), data_loader_(catalogue), query_processor_(handler) {
}

void JsonReader::LoadData(std::istream& input) {
    const json::Document doc = Load(input);
    const Dict& root = doc.GetRoot().AsMap();

    auto base_it = root.find("base_requests");
    if (base_it != root.end()) {
        const auto& base_requests = base_it->second.AsArray();
        data_loader_.LoadBaseRequest(base_requests);
    }

    auto stat_it = root.find("stat_requests");
    if (stat_it != root.end()) {
        stat_requests_ = stat_it->second.AsArray();
    }

    auto render_it = root.find("render_settings");
    if (render_it != root.end()) {
        render_settings_ = data_loader_.ParseRenderSettings(render_it->second.AsMap());
    }

    auto routing_it = root.find("routing_settings");
    if (routing_it != root.end()) {
        RoutingSettings rs = data_loader_.ParseRoutingSettings(routing_it->second.AsMap());
        handler_.SetRoutingSettings(rs);
    }
}

void JsonReader::ProcessRequests(std::ostream& output) const {
    if (!stat_requests_.has_value()) {
        json::Document empty_doc{json::Builder{}.StartArray().EndArray().Build()};
        json::Print(empty_doc, output);
        return;
    }
    auto answers = query_processor_.ProcessRequests(stat_requests_.value(), render_settings_);
    json::Document doc(std::move(answers));
    json::Print(doc, output);
}

}  // namespace transport