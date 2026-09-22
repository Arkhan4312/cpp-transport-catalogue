#pragma once
#include <optional>

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace transport {

class QueryProcessor {
public:
    explicit QueryProcessor(const RequestHandler& handler);

    json::Array ProcessRequests(const json::Array& stat_requests, const std::optional<RenderSettings>& settings) const;

private:
    const RequestHandler& handler_;

    json::Node MakeBusResponse(const json::Dict& request) const;
    json::Node MakeStopResponse(const json::Dict& request) const;
    json::Node MakeRouteResponse(const json::Dict& request) const;
};
}  // namespace transport