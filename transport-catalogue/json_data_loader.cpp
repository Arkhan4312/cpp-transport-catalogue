#include "json_data_loader.h"

namespace transport {
using namespace json;
JsonDataLoader::JsonDataLoader(TransportCatalogue& catalogue) : catalogue_(catalogue) {
}

void JsonDataLoader::LoadBaseRequest(const Array& base_requests) {
    for (const Node& node : base_requests) {
        const Dict& req = node.AsMap();
        const std::string& type = req.at("type").AsString();
        if (type == "Stop") {
            AddStop(req);
        }
    }

    for (const Node& node : base_requests) {
        const Dict& req = node.AsMap();
        const std::string& type = req.at("type").AsString();
        if (type == "Stop") {
            AddDistances(req);
        }
    }

    for (const Node& node : base_requests) {
        const Dict& req = node.AsMap();
        const std::string& type = req.at("type").AsString();
        if (type == "Bus") {
            AddBus(req);
        }
    }
}
RenderSettings JsonDataLoader::ParseRenderSettings(const json::Dict& settings_dict) const {
    RenderSettings settings;
    settings.width = settings_dict.at("width").AsDouble();
    settings.height = settings_dict.at("height").AsDouble();
    settings.padding = settings_dict.at("padding").AsDouble();
    settings.line_width = settings_dict.at("line_width").AsDouble();
    settings.stop_radius = settings_dict.at("stop_radius").AsDouble();
    settings.bus_label_font_size = settings_dict.at("bus_label_font_size").AsInt();
    const auto& bus_offset = settings_dict.at("bus_label_offset").AsArray();
    settings.bus_label_offset = {bus_offset[0].AsDouble(), bus_offset[1].AsDouble()};
    settings.stop_label_font_size = settings_dict.at("stop_label_font_size").AsInt();
    const auto& stop_offset = settings_dict.at("stop_label_offset").AsArray();
    settings.stop_label_offset = {stop_offset[0].AsDouble(), stop_offset[1].AsDouble()};
    settings.underlayer_color = ParseColor(settings_dict.at("underlayer_color"));
    settings.underlayer_width = settings_dict.at("underlayer_width").AsDouble();
    const auto& palette = settings_dict.at("color_palette").AsArray();
    settings.color_palette.reserve(palette.size());
    for (const auto& color_node : palette) {
        settings.color_palette.push_back(ParseColor(color_node));
    }
    return settings;
}
void JsonDataLoader::AddStop(const json::Dict& stop_node) {
    const std::string& name = stop_node.at("name").AsString();
    double lat = stop_node.at("latitude").AsDouble();
    double lng = stop_node.at("longitude").AsDouble();
    catalogue_.AddStop(std::string(name), {lat, lng});
}
void JsonDataLoader::AddDistances(const json::Dict& stop_node) {
    const std::string& from_name = stop_node.at("name").AsString();
    const Stop* from = catalogue_.FindStop(from_name);
    if (!from) {
        return;
    }
    const Dict& distances = stop_node.at("road_distances").AsMap();
    for (const auto& [to_name, dist_node] : distances) {
        const Stop* to = catalogue_.FindStop(to_name);
        if (to) {
            catalogue_.SetDistance(from, to, dist_node.AsDouble());
        }
    }
}
void JsonDataLoader::AddBus(const json::Dict& bus_node) {
    const std::string& name = bus_node.at("name").AsString();
    bool is_roundtrip = bus_node.at("is_roundtrip").AsBool();

    const Array& stops_array = bus_node.at("stops").AsArray();
    std::vector<std::string_view> stop_names;
    stop_names.reserve(stops_array.size());
    for (const Node& stop_node : stops_array) {
        stop_names.push_back(stop_node.AsString());
    }
    catalogue_.AddBus(std::string(name), stop_names, is_roundtrip);
}
svg::Color JsonDataLoader::ParseColor(const json::Node& node) const {
    if (node.IsString()) {
        return node.AsString();
    } else if (node.IsArray()) {
        const auto& arr = node.AsArray();
        if (arr.size() == 3) {
            int r = arr[0].AsInt();
            int g = arr[1].AsInt();
            int b = arr[2].AsInt();

            return svg::Rgb{r, g, b};
        } else if (arr.size() == 4) {
            int r = arr[0].AsInt();
            int g = arr[1].AsInt();
            int b = arr[2].AsInt();
            double a = arr[3].AsDouble();
            return svg::Rgba{r, g, b, a};
        } else {
            throw std::logic_error("Invalid color format");
        }
    } else {
        throw std::logic_error("Invalid color node");
    }
}
}  // namespace transport