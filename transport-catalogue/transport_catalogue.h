#pragma once
#include <deque>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "domain.h"
#include "geo.h"
namespace transport {

class TransportCatalogue {
public:
    void AddStop(std::string name, Coordinates coords);

    void AddBus(std::string name, const std::vector<std::string_view>& stop_names, bool is_ring);

    const Stop* FindStop(std::string_view name) const;

    const Bus* FindBus(std::string_view name) const;

    RouteInfo GetRouteInfo(const Bus* bus) const;

    const std::unordered_set<std::string_view>* GetBusesForStop(const Stop* stop) const;

    void SetDistance(const Stop* from, const Stop* to, double distance);

    double GetDistance(const Stop* from, const Stop* to) const;

    std::vector<const Bus*> GetAllBuses() const;

    std::vector<const Stop*> GetAllStops() const;

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