#include "transport_catalogue.h"

#include "geo.h"
namespace transport {
namespace detail {
std::vector<const Stop*> BuildFullRoute(const Bus* bus) {
    if (!bus || bus->stops.empty()) {
        return {};
    }
    std::vector<const Stop*> route;
    size_t reserve_size = bus->is_ring ? bus->stops.size() + 1 : bus->stops.size() * 2 - 1;
    route.reserve(reserve_size);
    route.insert(route.end(), bus->stops.begin(), bus->stops.end());
    if (!bus->is_ring) {
        for (auto it = bus->stops.rbegin() + 1; it != bus->stops.rend(); ++it) {
            route.push_back(*it);
        }
    }
    return route;
}

size_t CountUniqueStops(const std::vector<const Stop*>& route) {
    std::unordered_set<std::string_view> unique_names;
    for (const Stop* stop : route) {
        unique_names.insert(stop->name);
    }
    return unique_names.size();
}

RouteLengths ComputeLengths(const std::vector<const Stop*>& route, const TransportCatalogue& catalogue) {
    double route_length = 0.0;
    double geo_length = 0.0;
    for (size_t i = 1; i < route.size(); ++i) {
        route_length += catalogue.GetDistance(route[i - 1], route[i]);
        geo_length += ComputeDistance(route[i - 1]->coordinates, route[i]->coordinates);
    }
    return {route_length, geo_length};
}
}  // namespace detail

void TransportCatalogue::AddStop(std::string name, Coordinates coords) {
    stops_.push_back({std::move(name), coords});
    Stop& new_stop = stops_.back();
    stop_names_[new_stop.name] = &new_stop;
}

void TransportCatalogue::AddBus(std::string name, const std::vector<std::string_view>& stop_names, bool is_ring) {
    Bus bus;
    bus.name = std::move(name);
    bus.is_ring = is_ring;
    bus.stops.reserve(stop_names.size());
    for (const auto& stop_name : stop_names) {
        const Stop* stop = FindStop(stop_name);
        if (stop == nullptr) {
            continue;
        }
        bus.stops.push_back(stop);
    }
    if (bus.stops.empty()) {
        return;
    }
    buses_.push_back(std::move(bus));
    const Bus& new_bus = buses_.back();
    bus_names_[new_bus.name] = &new_bus;

    for (const Stop* stop : new_bus.stops) {
        stop_to_buses_[stop].insert(std::string_view(new_bus.name));
    }
}

const Stop* TransportCatalogue::FindStop(std::string_view name) const {
    auto it = stop_names_.find(name);
    if (it != stop_names_.end()) {
        return it->second;
    }
    return nullptr;
}

const Bus* TransportCatalogue::FindBus(std::string_view name) const {
    auto it = bus_names_.find(name);
    if (it != bus_names_.end()) {
        return it->second;
    }
    return nullptr;
}

RouteInfo TransportCatalogue::GetRouteInfo(const Bus* bus) const {
    auto route = detail::BuildFullRoute(bus);
    if (route.empty()) {
        return {0, 0, 0.0, 0.0};
    }
    size_t stops_count = route.size();
    size_t unique_stops = detail::CountUniqueStops(route);
    auto lengths = detail::ComputeLengths(route, *this);

    double curvature = (lengths.geo == 0.0) ? 1.0 : lengths.road / lengths.geo;
    return RouteInfo{stops_count, unique_stops, lengths.road, curvature};
}

const std::unordered_set<std::string_view>* TransportCatalogue::GetBusesForStop(const Stop* stop) const {
    if (!stop) {
        return nullptr;
    }
    auto it = stop_to_buses_.find(stop);
    if (it != stop_to_buses_.end()) {
        return &it->second;
    }
    return nullptr;
}

void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, double distance) {
    distances_[{from, to}] = distance;
}

double TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    auto it = distances_.find({from, to});
    if (it != distances_.end()) {
        return it->second;
    }
    auto it_rev = distances_.find({to, from});
    if (it_rev != distances_.end()) {
        return it_rev->second;
    }
    return 0.0;
}
std::vector<const Bus*> TransportCatalogue::GetAllBuses() const {
    std::vector<const Bus*> result;
    result.reserve(bus_names_.size());
    for (const auto& [name, bus] : bus_names_) {
        result.push_back(bus);
    }
    return result;
}
std::vector<const Stop*> TransportCatalogue::GetAllStops() const {
    std::vector<const Stop*> result;
    result.reserve(stop_names_.size());
    for (const auto& [name, stop] : stop_names_) {
        result.push_back(stop);
    }
    return result;
}
}  // namespace transport