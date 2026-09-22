#pragma once

#include <iostream>
#include <optional>

#include "json.h"
#include "json_data_loader.h"
#include "map_renderer.h"
#include "query_processor.h"
#include "request_handler.h"
namespace transport {

class JsonReader {
public:
    explicit JsonReader(TransportCatalogue& catalogue, RequestHandler& handler);

    void LoadData(std::istream& input);

    void ProcessRequests(std::ostream& output) const;

private:
    RequestHandler& handler_;
    JsonDataLoader data_loader_;
    QueryProcessor query_processor_;
    std::optional<RenderSettings> render_settings_;
    std::optional<json::Array> stat_requests_;
};

}  // namespace transport