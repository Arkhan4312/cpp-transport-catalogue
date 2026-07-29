#pragma once
#include <deque>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "geo.h"
namespace transport {
struct Stop {
    std::string name;
    Coordinates coordinates;
};

struct Bus {
    std::string name;
    bool is_ring = false;
    std::vector<const Stop*> stops;
};

struct RouteLengths {
    double road;
    double geo;
};

class TransportCatalogue {
public:
    struct RouteInfo {
        size_t stops_count;
        size_t unique_stops;
        double route_length;
        double curvature;
    };

    void AddStop(std::string name, Coordinates coords);

    void AddBus(std::string name, const std::vector<std::string_view>& stop_names, bool is_ring);

    const Stop* FindStop(std::string_view name) const;

    const Bus* FindBus(std::string_view name) const;

    RouteInfo GetRouteInfo(const Bus* bus) const;

    const std::unordered_set<std::string_view>* GetBusesForStop(const Stop* stop) const;

    void SetDistance(const Stop* from, const Stop* to, double distance);

    double GetDistance(const Stop* from, const Stop* to) const;

private:
    struct PairHash {
        size_t operator()(const std::pair<const Stop*, const Stop*>& p) const noexcept {
            size_t h1 = std::hash<const void*>{}(p.first);
            size_t h2 = std::hash<const void*>{}(p.second);
            return h1 ^ (h2 << 1);
        }
    };

    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, const Stop*> stop_names_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Bus*> bus_names_;
    std::unordered_map<const Stop*, std::unordered_set<std::string_view>> stop_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, PairHash> distances_;
};
}  // namespace transport