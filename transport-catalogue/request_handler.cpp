#include "request_handler.h"

#include <algorithm>
#include <sstream>

#include "map_renderer.h"
namespace transport {

RequestHandler::RequestHandler(const TransportCatalogue& catalogue) : catalogue_(catalogue) {
}

std::optional<RouteInfo> RequestHandler::GetBusInfo(std::string_view bus_name) const {
    const Bus* bus = catalogue_.FindBus(bus_name);
    if (!bus) {
        return std::nullopt;
    }
    return catalogue_.GetRouteInfo(bus);
}

std::vector<std::string> RequestHandler::GetBusesForStop(std::string_view stop_name) const {
    const Stop* stop = catalogue_.FindStop(stop_name);
    if (!stop) {
        return {};
    }
    const auto* buses_set = catalogue_.GetBusesForStop(stop);
    if (!buses_set) {
        return {};
    }
    std::vector<std::string> result;
    result.reserve(buses_set->size());
    for (std::string_view name : *buses_set) {
        result.emplace_back(name);
    }
    std::sort(result.begin(), result.end());
    return result;
}

bool RequestHandler::HasStop(std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name) != nullptr;
}

bool RequestHandler::HasBus(std::string_view bus_name) const {
    return catalogue_.FindBus(bus_name) != nullptr;
}
std::string RequestHandler::RenderMap(const RenderSettings& settings) const {
    auto buses = catalogue_.GetAllBuses();
    auto stops = catalogue_.GetAllStops();
    MapRenderer renderer(buses, stops, settings);
    svg::Document doc = renderer.Render();
    std::ostringstream out;
    doc.Render(out);
    return out.str();
}
}  // namespace transport