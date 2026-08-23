#pragma once

#include <iostream>
#include <optional>

#include "json.h"
#include "json_data_loader.h"
#include "map_renderer.h"
#include "map_saver.h"
#include "query_processor.h"
#include "request_handler.h"
#include "transport_catalogue.h"
namespace transport {

class JsonReader {
public:
    explicit JsonReader(TransportCatalogue& catalogue, const RequestHandler& handler);

    void LoadData(std::istream& input);

    void ProcessRequests(std::ostream& output) const;

    RenderSettings GetRenderSettings() const;

    void MapToFile(const std::string& filename) const;

private:
    TransportCatalogue& catalogue_;
    const RequestHandler& handler_;
    std::unique_ptr<JsonDataLoader> data_loader_;
    std::unique_ptr<QueryProcessor> query_processor_;
    std::optional<json::Document> doc_;
    std::optional<RenderSettings> render_settings_;
};

}  // namespace transport