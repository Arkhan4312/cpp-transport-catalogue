#include "domain.h"

namespace transport {
std::vector<const Stop*> BuildFullRoute(const Bus& bus) {
    if (bus.stops.empty()) {
        return {};
    }

    std::vector<const Stop*> route;
    route.reserve(bus.is_ring ? bus.stops.size() + 1 : bus.stops.size() * 2 - 1);
    route.insert(route.end(), bus.stops.begin(), bus.stops.end());

    if (!bus.is_ring) {
        for (auto it = bus.stops.rbegin() + 1; it != bus.stops.rend(); ++it) {
            route.push_back(*it);
        }
    }
    return route;
}
}  // namespace transport
