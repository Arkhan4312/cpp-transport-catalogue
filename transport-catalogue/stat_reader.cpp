#include "stat_reader.h"
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
} // detail namespace
void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
    std::ostream& output) {
    if (request.substr(0, 4) == "Bus ") {
        std::string bus_name = std::string(detail::TrimStat(request.substr(4)));
        const auto* bus = transport_catalogue.FindBus(bus_name);
        if (bus == nullptr) {
            output << "Bus " << bus_name << ": not found\n";
            return;
        }
        auto info = transport_catalogue.GetRouteInfo(bus);
        output << "Bus " << bus_name << ": "
        << info.stops_count << " stops on route, "
        << info.unique_stops << " unique stops, "
        << std::fixed << std::setprecision(6) << info.route_length << " route length\n";
        return;
    }

    if (request.substr(0,5) == "Stop ") {
        std::string stop_name = std::string(detail::TrimStat(request.substr(5)));
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
        for (const auto& bus : *buses_set) {
            output << " " << bus;
        }
        output << "\n";
        return;
    }
}
} // namespace transport