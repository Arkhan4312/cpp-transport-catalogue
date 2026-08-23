#include "query_processor.h"
namespace transport {
using namespace json;
QueryProcessor::QueryProcessor(const RequestHandler& handler) : handler_(handler) {
}

Array QueryProcessor::ProcessRequests(const Array& stat_requests, const RenderSettings& settings) const {
    Array answers;
    answers.reserve(stat_requests.size());

    for (const Node& req_node : stat_requests) {
        const Dict& req = req_node.AsMap();
        std::string type = req.at("type").AsString();

        Dict answer;
        answer["request_id"] = req.at("id").AsInt();

        if (type == "Bus") {
            answer = MakeBusResponse(req);
        } else if (type == "Stop") {
            answer = MakeStopResponse(req);
        } else if (type == "Map") {
            answer["map"] = handler_.RenderMap(settings);
        }

        answers.emplace_back(std::move(answer));
    }
    return answers;
}

Dict QueryProcessor::MakeBusResponse(const Dict& request) const {
    Dict answer;
    std::string bus_name = request.at("name").AsString();
    int request_id = request.at("id").AsInt();
    answer["request_id"] = request_id;

    auto info = handler_.GetBusInfo(bus_name);
    if (!info) {
        answer["error_message"] = std::string("not found");
        return answer;
    }

    answer["curvature"] = info->curvature;
    answer["route_length"] = info->route_length;
    answer["stop_count"] = static_cast<int>(info->stops_count);
    answer["unique_stop_count"] = static_cast<int>(info->unique_stops);
    return answer;
}

Dict QueryProcessor::MakeStopResponse(const Dict& request) const {
    Dict answer;
    std::string stop_name = request.at("name").AsString();
    int request_id = request.at("id").AsInt();
    answer["request_id"] = request_id;

    if (!handler_.HasStop(stop_name)) {
        answer["error_message"] = std::string("not found");
        return answer;
    }

    auto buses = handler_.GetBusesForStop(stop_name);
    Array buses_array;
    buses_array.reserve(buses.size());
    for (const auto& bus : buses) {
        buses_array.emplace_back(bus);
    }
    answer["buses"] = std::move(buses_array);
    return answer;
}
}  // namespace transport