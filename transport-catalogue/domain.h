#pragma once
#include <string>
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

struct RouteInfo {
    size_t stops_count;
    size_t unique_stops;
    double route_length;
    double curvature;
};

}  // namespace transport
