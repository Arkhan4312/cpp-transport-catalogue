#include "input_reader.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
namespace transport {
namespace detail {
/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для любого маршрута возвращает массив названий остановок
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }
    return Split(route, '-');
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

std::pair<std::string_view, double> ParseDistance(std::string_view part) {
    size_t pos = part.find("m to");
    if (pos == std::string_view::npos) {
        return { "", 0.0 };
    }
    std::string_view num_part = part.substr(0, pos);
    size_t start_num = num_part.find_first_not_of(' ');
    if (start_num == std::string_view::npos) {
        return { "",0.0 };
    }
    num_part = num_part.substr(start_num);
    double distance = std::stod(std::string(num_part));
    std::string_view stop_name = Trim(part.substr(pos + 4));
    return { stop_name,distance };
}
} // detail namespace

void InputReader::ParseLine(std::string_view line) {
    auto command_description = detail::ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::AddStops(TransportCatalogue& catalogue) const {
    for(const auto& cmd : commands_) {
        if (cmd.command == "Stop") {
            auto coords = detail::ParseCoordinates(cmd.description);
            catalogue.AddStop(cmd.id, coords);
        }
    }
}

void InputReader::AddDistances(TransportCatalogue& catalogue) const {
    for (const auto& cmd : commands_) {
        if (cmd.command != "Stop") {
            continue;
            }
        auto parts = detail::Split(cmd.description, ',');
        if (parts.size() < 3 ) {
            continue;
            }
        for (size_t i = 2; i < parts.size(); ++i) {
            auto [stop_name, distance] = detail::ParseDistance(parts[i]);
            if (stop_name.empty()) {
                continue;
            }
            const Stop* from = catalogue.FindStop(cmd.id);
            const Stop* to = catalogue.FindStop(stop_name);
            if (from && to) {
                catalogue.SetDistance(from, to, distance);
            }
        }
    }
}

void InputReader::AddBusses(TransportCatalogue& catalogue) const {
    for (const auto& cmd : commands_) {
        if (cmd.command == "Bus") {
            auto route_stops_view = detail::ParseRoute(cmd.description);
            bool is_ring = (cmd.description.find('>') != std::string::npos);
            std::vector<std::string> stop_names;
            stop_names.reserve(route_stops_view.size());
            for (auto& sv : route_stops_view) {
                stop_names.emplace_back(sv);
            }
            catalogue.AddBus(cmd.id, std::move(stop_names), is_ring);
        }
    }
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
    AddStops(catalogue);
    AddDistances(catalogue);
    AddBusses(catalogue);
}

void InputReader::ReadRequest(std::istream& input) { 
    int base_request_count;
    input >> base_request_count >> std::ws;
    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(input, line);
        ParseLine(line);
    }
}
} // namespace transport
