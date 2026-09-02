#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

namespace transport {
class RequestHandler {
public:
    explicit RequestHandler(const TransportCatalogue& catalogue);

    std::optional<RouteInfo> GetBusInfo(std::string_view bus_name) const;
    std::vector<std::string_view> GetBusesForStop(std::string_view stop_name) const;
    bool HasStop(std::string_view stop_name) const;

    std::string RenderMap(const RenderSettings& settings) const;
    const TransportCatalogue& GetCatalogue() const;

private:
    const TransportCatalogue& catalogue_;
};
}  // namespace transport