#include "map_renderer.h"

#include <algorithm>
#include <limits>
#include <unordered_set>
namespace transport {
using namespace svg;

namespace {

std::vector<const Stop*> BuildFullRouteForRender(const Bus* bus) {
    if (!bus || bus->stops.empty()) {
        return {};
    }
    std::vector<const Stop*> route;
    route.reserve(bus->is_ring ? bus->stops.size() + 1 : bus->stops.size() * 2 - 1);
    route.insert(route.end(), bus->stops.begin(), bus->stops.end());
    if (bus->is_ring) {
        if (bus->stops.front() != bus->stops.back()) {
            route.push_back(bus->stops.front());
        }
    } else {
        for (auto it = bus->stops.rbegin() + 1; it != bus->stops.rend(); ++it) {
            route.push_back(*it);
        }
    }
    return route;
}
}  // namespace

SphereProjector::SphereProjector(const std::vector<const Stop*>& stops, double width, double height, double padding)
    : padding_(padding) {
    if (stops.empty()) {
        return;
    }
    double min_lat = std::numeric_limits<double>::max();
    double max_lat = std::numeric_limits<double>::lowest();
    double min_lng = std::numeric_limits<double>::max();
    double max_lng = std::numeric_limits<double>::lowest();

    for (const Stop* stop : stops) {
        min_lat = std::min(min_lat, stop->coordinates.lat);
        max_lat = std::max(max_lat, stop->coordinates.lat);
        min_lng = std::min(min_lng, stop->coordinates.lng);
        max_lng = std::max(max_lng, stop->coordinates.lng);
    }
    min_lat_ = min_lat;
    max_lat_ = max_lat;
    min_lng_ = min_lng;
    max_lng_ = max_lng;
    double x_span = max_lng - min_lng;
    double y_span = max_lat - min_lat;
    if (x_span == 0 && y_span == 0) {
        zoom_coef_ = 1.0;
    } else {
        double x_ratio = (width - 2 * padding) / (x_span > 0 ? x_span : 1.0);
        double y_ratio = (height - 2 * padding) / (y_span > 0 ? y_span : 1.0);
        zoom_coef_ = std::min(x_ratio, y_ratio);
    }
}

Point SphereProjector::operator()(Coordinates coords) const {
    double x = (coords.lng - min_lng_) * zoom_coef_ + padding_;
    double y = (max_lat_ - coords.lat) * zoom_coef_ + padding_;
    return {x, y};
}

MapRenderer::MapRenderer(const std::vector<const Bus*>& buses, const std::vector<const Stop*>& stops,
                         const RenderSettings& settings)
    : buses_(buses), stops_(stops), settings_(settings) {
}

svg::Document MapRenderer::Render() const {
    auto all_buses = buses_;
    std::sort(all_buses.begin(), all_buses.end(), [](const Bus* lhs, const Bus* rhs) { return lhs->name < rhs->name; });

    std::unordered_set<const Stop*> unique_stops_set;
    for (const Bus* bus : all_buses) {
        for (const Stop* stop : bus->stops) {
            unique_stops_set.insert(stop);
        }
    }
    std::vector<const Stop*> unique_stops(unique_stops_set.begin(), unique_stops_set.end());
    std::sort(unique_stops.begin(), unique_stops.end(),
              [](const Stop* lhs, const Stop* rhs) { return lhs->name < rhs->name; });
    if (unique_stops.empty()) {
        Document doc;
        return doc;
    }

    SphereProjector projector(unique_stops, settings_.width, settings_.height, settings_.padding);
    Document doc;
    const auto& palette = settings_.color_palette;
    size_t palette_size = palette.size();
    if (palette_size == 0) {
        return doc;
    }

    std::vector<BusLabelInfo> bus_labels;

    for (size_t i = 0; i < all_buses.size(); ++i) {
        const Bus* bus = all_buses[i];
        if (bus->stops.empty()) {
            continue;
        }

        const std::string& color = palette[i % palette_size];

        auto route = BuildFullRouteForRender(bus);
        if (route.empty()) {
            continue;
        }

        Polyline polyline;
        polyline.SetFillColor(NoneColor)
            .SetStrokeColor(color)
            .SetStrokeWidth(settings_.line_width)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND);
        for (const Stop* stop : route) {
            polyline.AddPoint(projector(stop->coordinates));
        }
        doc.Add(std::move(polyline));

        std::vector<const Stop*> end_stops;
        if (bus->is_ring) {
            end_stops.push_back(bus->stops.front());
        } else {
            end_stops.push_back(bus->stops.front());
            end_stops.push_back(bus->stops.back());
            if (end_stops[0] == end_stops[1]) {
                end_stops.pop_back();
            }
        }
        for (const Stop* stop : end_stops) {
            bus_labels.push_back({projector(stop->coordinates), bus->name, color});
        }
    }

    for (const auto& info : bus_labels) {
        Text underlayer;
        underlayer.SetPosition(info.point)
            .SetOffset(settings_.bus_label_offset)
            .SetFontSize(settings_.bus_label_font_size)
            .SetFontFamily(std::string("Verdana"))
            .SetFontWeight(std::string("bold"))
            .SetData(info.text)
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND);
        doc.Add(std::move(underlayer));
        Text main_text;
        main_text.SetPosition(info.point)
            .SetOffset(settings_.bus_label_offset)
            .SetFontSize(settings_.bus_label_font_size)
            .SetFontFamily(std::string("Verdana"))
            .SetFontWeight(std::string("bold"))
            .SetData(info.text)
            .SetFillColor(info.color);
        doc.Add(std::move(main_text));
    }
    for (const Stop* stop : unique_stops) {
        Point p = projector(stop->coordinates);

        Circle circle;
        circle.SetCenter(p).SetRadius(settings_.stop_radius).SetFillColor(std::string("white"));
        doc.Add(std::move(circle));
    }
    for (const Stop* stop : unique_stops) {
        Point p = projector(stop->coordinates);

        Text underlayer_text;
        underlayer_text.SetPosition(p)
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily(std::string("Verdana"))
            .SetData(stop->name)
            .SetFillColor(settings_.underlayer_color)
            .SetStrokeColor(settings_.underlayer_color)
            .SetStrokeWidth(settings_.underlayer_width)
            .SetStrokeLineCap(StrokeLineCap::ROUND)
            .SetStrokeLineJoin(StrokeLineJoin::ROUND);
        doc.Add(std::move(underlayer_text));

        Text main_text;
        main_text.SetPosition(p)
            .SetOffset(settings_.stop_label_offset)
            .SetFontSize(settings_.stop_label_font_size)
            .SetFontFamily(std::string("Verdana"))
            .SetData(stop->name)
            .SetFillColor(std::string("black"));
        doc.Add(std::move(main_text));
    }

    return doc;
}

}  // namespace transport
