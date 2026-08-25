#pragma once
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace transport {

class JsonDataLoader {
public:
    explicit JsonDataLoader(TransportCatalogue& catalogue);

    void LoadBaseRequest(const json::Array& base_requests);

    RenderSettings ParseRenderSettings(const json::Dict& settings_dict) const;

private:
    TransportCatalogue& catalogue_;

    void AddStop(const json::Dict& stop_node);
    void AddDistances(const json::Dict& stop_node);
    void AddBus(const json::Dict& bus_node);
    svg::Color ParseColor(const json::Node& node) const;
};
}  // namespace transport