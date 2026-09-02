#include "query_processor.h"

#include "json_builder.h"

namespace transport {
using namespace json;
QueryProcessor::QueryProcessor(const RequestHandler& handler) : handler_(handler) {
}

Array QueryProcessor::ProcessRequests(const Array& stat_requests, const std::optional<RenderSettings>& settings) const {
    Array answers;
    answers.reserve(stat_requests.size());

    for (const Node& req_node : stat_requests) {
        const Dict& req = req_node.AsMap();
        const std::string& type = req.at("type").AsString();

        if (type == "Bus") {
            answers.emplace_back(MakeBusResponse(req));
        } else if (type == "Stop") {
            answers.emplace_back(MakeStopResponse(req));
        } else if (type == "Map") {
            Node map_response = Builder{}
                                    .StartDict()
                                    .Key("request_id")
                                    .Value(req.at("id").AsInt())
                                    .Key("map")
                                    .Value(settings.has_value() ? handler_.RenderMap(settings.value()) : "")
                                    .EndDict()
                                    .Build();
            answers.emplace_back(std::move(map_response));
        }
    }
    return answers;
}

Dict QueryProcessor::MakeBusResponse(const Dict& request) const {
    const std::string& bus_name = request.at("name").AsString();

    auto info = handler_.GetBusInfo(bus_name);

    if (!info) {
        Node node = Builder{}
                        .StartDict()
                        .Key("request_id")
                        .Value(request.at("id").AsInt())
                        .Key("error_message")
                        .Value("not found")
                        .EndDict()
                        .Build();
        return std::get<Dict>(node.GetValue());
    }
    Node node = Builder{}
                    .StartDict()
                    .Key("request_id")
                    .Value(request.at("id").AsInt())
                    .Key("curvature")
                    .Value(info->curvature)
                    .Key("route_length")
                    .Value(info->route_length)
                    .Key("stop_count")
                    .Value(static_cast<int>(info->stops_count))
                    .Key("unique_stop_count")
                    .Value(static_cast<int>(info->unique_stops))
                    .EndDict()
                    .Build();
    return std::get<Dict>(node.GetValue());
}

Dict QueryProcessor::MakeStopResponse(const Dict& request) const {
    const std::string& stop_name = request.at("name").AsString();

    if (!handler_.HasStop(stop_name)) {
        Node node = Builder{}
                        .StartDict()
                        .Key("request_id")
                        .Value(request.at("id").AsInt())
                        .Key("error_message")
                        .Value("not found")
                        .EndDict()
                        .Build();
        return std::get<Dict>(node.GetValue());
    }

    auto buses = handler_.GetBusesForStop(stop_name);
    Builder builder;
    builder.StartDict().Key("request_id").Value(request.at("id").AsInt()).Key("buses").StartArray();
    for (const auto& bus : buses) {
        builder.Value(std::string(bus));
    }
    Node node = builder.EndArray().EndDict().Build();
    return std::get<Dict>(node.GetValue());
}
}  // namespace transport