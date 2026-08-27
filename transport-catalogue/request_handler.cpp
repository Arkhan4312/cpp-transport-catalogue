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

std::vector<std::string_view> RequestHandler::GetBusesForStop(std::string_view stop_name) const {
    const Stop* stop = catalogue_.FindStop(stop_name);
    if (!stop) {
        return {};
    }
    const auto& buses_set = catalogue_.GetBusesForStop(stop);

    std::vector<std::string_view> result;
    result.reserve(buses_set.size());
    for (std::string_view name : buses_set) {
        result.push_back(name);
    }
    std::sort(result.begin(), result.end());
    return result;
}

bool RequestHandler::HasStop(std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name) != nullptr;
}

std::string RequestHandler::RenderMap(const RenderSettings& settings) const {
    const auto& buses = catalogue_.GetAllBuses();
    std::vector<const Bus*> bus_ptrs;
    bus_ptrs.reserve(buses.size());
    for (const auto& bus : buses) {
        bus_ptrs.push_back(&bus);
    }
    MapRenderer renderer(bus_ptrs, settings);
    svg::Document doc = renderer.Render();
    std::ostringstream out;
    doc.Render(out);
    return out.str();
}
}  // namespace transport