#include "stat_reader.h"
#include <vector>
#include <algorithm>
#include <iomanip>
namespace transport {
namespace detail {
std::string_view TrimStat(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

struct StatQuery {
    std::string_view type;
    std::string_view name;
};

StatQuery ParseStatQuery(std::string_view request) {
    const size_t space_pos = request.find(' ');
    if (space_pos == std::string_view::npos) {
        return { request, {} };
    }
    std::string_view type = request.substr(0, space_pos);
    std::string_view name = request.substr(space_pos+1);
    name = TrimStat(name);
    return { type, name };
}

void PrintBusInfo(const TransportCatalogue& transport_catalogue, std::string_view bus_name, std::ostream& output) {
    const auto* bus = transport_catalogue.FindBus(bus_name);
    if (bus == nullptr) {
        output << "Bus " << bus_name << ": not found\n";
        return;
    }
    const auto info = transport_catalogue.GetRouteInfo(bus);
    output << "Bus " << bus_name << ": "
        << info.stops_count << " stops on route, "
        << info.unique_stops << " unique stops, "
        << std::fixed << std::setprecision(6) << info.route_length << " route length, "
        << std::fixed << std::setprecision(6) << info.curvature << " curvature\n";
}

void PrintStopInfo(const TransportCatalogue& transport_catalogue, std::string_view stop_name, std::ostream& output) {
    const auto* stop = transport_catalogue.FindStop(stop_name);
    if (stop == nullptr) {
        output << "Stop " << stop_name << ": not found\n";
        return;
    }
    const auto* buses_set = transport_catalogue.GetBusesForStop(stop);
    if (buses_set == nullptr || buses_set->empty()) {
        output << "Stop " << stop_name << ": no buses\n";
        return;
    }
    output << "Stop " << stop_name << ": buses";
    std::vector<std::string_view> sorted_buses(buses_set->begin(), buses_set->end());
    std::sort(sorted_buses.begin(), sorted_buses.end());
    for (std::string_view bus : sorted_buses) {
        output << " " << bus;
    }
    output << "\n";
}
} // namespace detail

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    const auto parsed = detail::ParseStatQuery(request);
    if (parsed.type == "Bus") {
        detail::PrintBusInfo(transport_catalogue, parsed.name, output);
    } else if (parsed.type == "Stop") {
        detail::PrintStopInfo(transport_catalogue, parsed.name, output);
    }
}

void ProcessStatRequest(std::istream& input, std::ostream& output, const TransportCatalogue& catalogue) {
    size_t stat_request_count;
    input >> stat_request_count >> std::ws;
    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}
} // namespace transport
