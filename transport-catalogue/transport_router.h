#pragma once

#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport {

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

struct RouteItem {
    enum class Type {
        Wait,
        Bus,
    };
    Type type = Type::Wait;
    std::string_view stop_name;
    std::string_view bus_name;
    int span_count = 0;
    double time = 0.0;
};

struct RouteResult {
    double total_time = 0.0;
    std::vector<RouteItem> items;
};

class TransportRouter {
public:
    TransportRouter(const TransportCatalogue& catalogue, const RoutingSettings& settings);

    std::optional<RouteResult> BuildRoute(std::string_view from, std::string_view to) const;

private:
    struct EdgeInfo {
        std::string_view bus_name;
        int span_count = 0;
        double time = 0.0;
    };

    void InitVertexIds();
    void BuildGraph();
    void AddBusEdges(const Bus& bus);
    void AddEdgesFromStop(const Bus& bus, const std::vector<const Stop*>& route,
                          const std::vector<double>& prefix_times, size_t from_idx);

    std::vector<double> ComputePrefixTimes(const std::vector<const Stop*>& route) const;

    std::optional<graph::VertexId> GetVertexId(std::string_view stop_name) const;
    std::vector<RouteItem> BuildItems(const std::vector<graph::EdgeId>& edges) const;

    const TransportCatalogue& catalogue_;
    RoutingSettings settings_;

    std::vector<const Stop*> id_to_stop_;
    std::unordered_map<const Stop*, graph::VertexId> stop_to_id_;

    graph::DirectedWeightedGraph<double> graph_;
    std::vector<EdgeInfo> edges_info_;
    std::unique_ptr<graph::Router<double>> router_;
};
}  // namespace transport