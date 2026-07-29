#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport {
struct CommandDescription {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }
    bool operator!() const {
        return !operator bool();
    }
    std::string command;      // Название команды
    std::string id;           // id маршрута или остановки
    std::string description;  // Параметры команды
};

struct ParsedDistance {
    std::string_view stop_name;
    double distance;
};

class InputReader {
public:
    /**
     * Парсит строку в структуру CommandDescription и сохраняет результат в
     * commands_
     */
    void ParseLine(std::string_view line);

    /**
     * Наполняет данными транспортный справочник, используя команды из commands_
     */

    void AddStops(TransportCatalogue& catalogue) const;
    void AddDistances(TransportCatalogue& catalogue) const;
    void AddBuses(TransportCatalogue& catalogue) const;
    void ApplyCommands(TransportCatalogue& catalogue) const;

    void ReadRequest(std::istream& input);

private:
    std::vector<CommandDescription> commands_;
};
}  // namespace transport