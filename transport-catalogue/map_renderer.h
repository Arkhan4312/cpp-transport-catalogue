#pragma once

#include <string>
#include <vector>

#include "domain.h"
#include "svg.h"
namespace transport {
struct RenderSettings {
    double width = 0.0;
    double height = 0.0;
    double padding = 0.0;
    double line_width = 0.0;
    double stop_radius = 0.0;
    int bus_label_font_size = 0;
    svg::Point bus_label_offset;
    int stop_label_font_size = 0;
    svg::Point stop_label_offset;
    std::string underlayer_color;
    double underlayer_width = 0.0;
    std::vector<std::string> color_palette;
};

class SphereProjector {
public:
    SphereProjector(const std::vector<const Stop*>& stops, double width, double height, double padding);
    svg::Point operator()(Coordinates coords) const;

private:
    double min_lat_ = 0.0;
    double max_lat_ = 0.0;
    double min_lng_ = 0.0;
    double max_lng_ = 0.0;
    double zoom_coef_ = 0.0;
    double padding_ = 0.0;
};

class MapRenderer {
public:
    MapRenderer(const std::vector<const Bus*>& buses, const std::vector<const Stop*>& stops, const RenderSettings& settings);
    svg::Document Render() const;

private:
    const std::vector<const Bus*> buses_;
    const std::vector<const Stop*> stops_;
    const RenderSettings& settings_;
};

}  // namespace transport