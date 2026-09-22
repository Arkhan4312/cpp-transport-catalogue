#include "transport_router.h"

namespace transport {

namespace {
constexpr double METERS_PER_KM = 1000.0;
constexpr double MINUTES_PER_HOUR = 60.0;
}  // namespace

TransportRouter::TransportRouter(const TransportCatalogue& catalogue, const RoutingSettings& settings)
    : catalogue_(catalogue), settings_(settings) {
    InitVertexIds();
    BuildGraph();
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

std::optional<RouteResult> TransportRouter::BuildRoute(std::string_view from, std::string_view to) const {
    const auto from_id = GetVertexId(from);
    const auto to_id = GetVertexId(to);
    if (!from_id || !to_id) {
        return std::nullopt;
    }
    const auto route = router_->BuildRoute(*from_id, *to_id);
    if (!route) {
        return std::nullopt;
    }

    return RouteResult{route->weight, BuildItems(route->edges)};
}

void TransportRouter::InitVertexIds() {
    const auto& stops = catalogue_.GetAllStops();
    id_to_stop_.reserve(stops.size());
    stop_to_id_.reserve(stops.size());

    for (const Stop& stop : catalogue_.GetAllStops()) {
        stop_to_id_[&stop] = id_to_stop_.size();
        id_to_stop_.push_back(&stop);
    }
}

void transport::TransportRouter::BuildGraph() {
    graph_ = graph::DirectedWeightedGraph<double>(id_to_stop_.size());
    for (const Bus& bus : catalogue_.GetAllBuses()) {
        AddBusEdges(bus);
    }
}

void TransportRouter::AddBusEdges(const Bus& bus) {
    if (bus.stops.empty()) {
        return;
    }
    const auto route = BuildFullRoute(bus);
    const auto prefix_times = ComputePrefixTimes(route);

    for (size_t i = 0; i + 1 < route.size(); ++i) {
        AddEdgesFromStop(bus, route, prefix_times, i);
    }
}

void TransportRouter::AddEdgesFromStop(const Bus& bus, const std::vector<const Stop*>& route,
                                       const std::vector<double>& prefix_times, size_t from_idx) {
    const auto from_it = stop_to_id_.find(route[from_idx]);
    if (from_it == stop_to_id_.end()) {
        return;
    }

    for (size_t to_idx = from_idx + 1; to_idx < route.size(); ++to_idx) {
        const auto to_it = stop_to_id_.find(route[to_idx]);
        if (to_it == stop_to_id_.end()) {
            continue;
        }

        const double bus_time = prefix_times[to_idx] - prefix_times[from_idx];
        const double weight = static_cast<double>(settings_.bus_wait_time) + bus_time;

        graph_.AddEdge({from_it->second, to_it->second, weight});

        edges_info_.push_back({bus.name, static_cast<int>(to_idx - from_idx), bus_time});
    }
}

std::vector<double> TransportRouter::ComputePrefixTimes(const std::vector<const Stop*>& route) const {
    std::vector<double> prefix(route.size(), 0.0);
    for (size_t i = 1; i < route.size(); ++i) {
        const double dist_m = catalogue_.GetDistance(route[i - 1], route[i]);
        const double time_min = dist_m / METERS_PER_KM / settings_.bus_velocity * MINUTES_PER_HOUR;
        prefix[i] = prefix[i - 1] + time_min;
    }
    return prefix;
}

std::optional<graph::VertexId> TransportRouter::GetVertexId(std::string_view stop_name) const {
    const Stop* stop = catalogue_.FindStop(stop_name);
    if (stop == nullptr) {
        return std::nullopt;
    }
    const auto it = stop_to_id_.find(stop);
    if (it == stop_to_id_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<RouteItem> TransportRouter::BuildItems(const std::vector<graph::EdgeId>& edges) const {
    std::vector<RouteItem> items;
    items.reserve(edges.size() * 2);

    for (const graph::EdgeId edge_id : edges) {
        const auto& edge = graph_.GetEdge(edge_id);
        const auto& info = edges_info_[edge_id];

        RouteItem wait;
        wait.type = RouteItem::Type::Wait;
        wait.stop_name = id_to_stop_[edge.from]->name;
        wait.time = static_cast<double>(settings_.bus_wait_time);
        items.push_back(wait);

        RouteItem bus;
        bus.type = RouteItem::Type::Bus;
        bus.bus_name = info.bus_name;
        bus.span_count = info.span_count;
        bus.time = info.time;
        items.push_back(bus);
    }
    return items;
}

}  // namespace transport