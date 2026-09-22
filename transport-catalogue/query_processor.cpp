#include "query_processor.h"

#include "json_builder.h"

namespace transport {
using namespace json;

namespace {
Node MakeErrorResponse(int id, const std::string& message) {
    return json::Builder{}
        .StartDict()
        .Key("request_id")
        .Value(id)
        .Key("error_message")
        .Value(message)
        .EndDict()
        .Build();
}

}  // namespace
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
        } else if (type == "Route") {
            answers.emplace_back(MakeRouteResponse(req));
        }
    }
    return answers;
}

Node QueryProcessor::MakeBusResponse(const Dict& request) const {
    const std::string& bus_name = request.at("name").AsString();
    int id = request.at("id").AsInt();

    auto info = handler_.GetBusInfo(bus_name);

    if (!info) {
        Node node = MakeErrorResponse(id, "not found");
        return std::get<Dict>(node.GetValue());
    }
    return Builder{}
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
}

Node QueryProcessor::MakeStopResponse(const Dict& request) const {
    const std::string& stop_name = request.at("name").AsString();
    int id = request.at("id").AsInt();

    if (!handler_.HasStop(stop_name)) {
        Node node = MakeErrorResponse(id, "not found");
        return std::get<Dict>(node.GetValue());
    }

    auto buses = handler_.GetBusesForStop(stop_name);
    Builder builder;
    builder.StartDict().Key("request_id").Value(request.at("id").AsInt()).Key("buses").StartArray();
    for (const auto& bus : buses) {
        builder.Value(std::string(bus));
    }
    return builder.EndArray().EndDict().Build();
}

Node QueryProcessor::MakeRouteResponse(const json::Dict& request) const {
    const std::string& from = request.at("from").AsString();
    const std::string& to = request.at("to").AsString();
    const int id = request.at("id").AsInt();

    auto route = handler_.GetRoute(from, to);
    if (!route) {
        Node node = MakeErrorResponse(id, "not found");
        return std::get<Dict>(node.GetValue());
    }

    Builder builder;
    builder.StartDict()
        .Key("request_id")
        .Value(id)
        .Key("total_time")
        .Value(route->total_time)
        .Key("items")
        .StartArray();

    for (const auto& item : route->items) {
        if (item.type == RouteItem::Type::Wait) {
            builder.StartDict()
                .Key("type")
                .Value(std::string("Wait"))
                .Key("stop_name")
                .Value(std::string(item.stop_name))
                .Key("time")
                .Value(static_cast<int>(item.time))
                .EndDict();
        } else {
            builder.StartDict()
                .Key("type")
                .Value(std::string("Bus"))
                .Key("bus")
                .Value(std::string(item.bus_name))
                .Key("span_count")
                .Value(item.span_count)
                .Key("time")
                .Value(item.time)
                .EndDict();
        }
    }

    return builder.EndArray().EndDict().Build();
}
}  // namespace transport