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

void AddLabelPair(svg::Document& doc, const Point& point, const std::string& text, int font_size, const Point& offset,
                  const Color& underlayer_color, double underlayer_width, const Color& text_color, bool bold = false) {
    Text underlayer;
    underlayer.SetPosition(point)
        .SetOffset(offset)
        .SetFontSize(font_size)
        .SetFontFamily(std::string("Verdana"))
        .SetData(text)
        .SetFillColor(underlayer_color)
        .SetStrokeColor(underlayer_color)
        .SetStrokeWidth(underlayer_width)
        .SetStrokeLineCap(StrokeLineCap::ROUND)
        .SetStrokeLineJoin(StrokeLineJoin::ROUND);
    if (bold) {
        underlayer.SetFontWeight(std::string("bold"));
    }

    doc.Add(std::move(underlayer));

    Text main_text;
    main_text.SetPosition(point)
        .SetOffset(offset)
        .SetFontSize(font_size)
        .SetFontFamily(std::string("Verdana"))
        .SetData(text)
        .SetFillColor(text_color);
    if (bold) {
        main_text.SetFontWeight(std::string("bold"));
    }

    doc.Add(std::move(main_text));
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

    static constexpr double eps = std::numeric_limits<double>::epsilon();
    double x_span = max_lng - min_lng;
    double y_span = max_lat - min_lat;

    double width_available = width - 2 * padding;
    double height_available = height - 2 * padding;

    bool x_zero = std::abs(x_span) < eps;
    bool y_zero = std::abs(y_span) < eps;

    if (x_zero && y_zero) {
        zoom_coef_ = 1.0;
    } else if (x_zero) {
        zoom_coef_ = height_available / y_span;
    } else if (y_zero) {
        zoom_coef_ = width_available / x_span;
    } else {
        double x_ratio = width_available / x_span;
        double y_ratio = height_available / y_span;
        zoom_coef_ = std::min(x_ratio, y_ratio);
    }
}

Point SphereProjector::operator()(Coordinates coords) const {
    double x = (coords.lng - min_lng_) * zoom_coef_ + padding_;
    double y = (max_lat_ - coords.lat) * zoom_coef_ + padding_;
    return {x, y};
}

MapRenderer::MapRenderer(const std::vector<const Bus*>& buses, const RenderSettings& settings)
    : buses_(buses), settings_(settings) {
}

PreparedData MapRenderer::PrepareData() const {
    PreparedData result;
    result.sorted_bus = buses_;
    std::sort(result.sorted_bus.begin(), result.sorted_bus.end(),
              [](const Bus* lhs, const Bus* rhs) { return lhs->name < rhs->name; });

    std::unordered_set<const Stop*> unique_stops_set;
    for (const Bus* bus : result.sorted_bus) {
        for (const Stop* stop : bus->stops) {
            unique_stops_set.insert(stop);
        }
    }
    result.unique_stops.assign(unique_stops_set.begin(), unique_stops_set.end());
    std::sort(result.unique_stops.begin(), result.unique_stops.end(),
              [](const Stop* lhs, const Stop* rhs) { return lhs->name < rhs->name; });
    return result;
}

SphereProjector MapRenderer::CreateProjector(const std::vector<const Stop*>& unique_stops) const {
    return SphereProjector(unique_stops, settings_.width, settings_.height, settings_.padding);
}

void MapRenderer::RenderBusLines(svg::Document& doc, const std::vector<const Bus*>& sorted_buses,
                                 const SphereProjector& projector) const {
    const auto& palette = settings_.color_palette;
    size_t palette_size = palette.size();
    if (palette_size == 0) {
        return;
    }

    for (size_t i = 0; i < sorted_buses.size(); ++i) {
        const Bus* bus = sorted_buses[i];
        if (bus->stops.empty()) {
            continue;
        }

        const Color& color = palette[i % palette_size];
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
    }
}

void MapRenderer::RenderBusLabels(svg::Document& doc, const std::vector<const Bus*>& sorted_buses,
                                  const SphereProjector& projector) const {
    const auto& palette = settings_.color_palette;
    size_t palette_size = palette.size();
    if (palette_size == 0) {
        return;
    }
    struct BusLabelInfo {
        Point point;
        std::string text;
        Color color;
    };
    std::vector<BusLabelInfo> bus_labels;

    for (size_t i = 0; i < sorted_buses.size(); ++i) {
        const Bus* bus = sorted_buses[i];
        if (bus->stops.empty()) {
            continue;
        }

        const Color& color = palette[i % palette_size];

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
        AddLabelPair(doc, info.point, info.text, settings_.bus_label_font_size, settings_.bus_label_offset,
                     settings_.underlayer_color, settings_.underlayer_width, info.color, true);
    }
}

void MapRenderer::RenderStopCircles(svg::Document& doc, const std::vector<const Stop*>& unique_stops,
                                    const SphereProjector& projector) const {
    for (const Stop* stop : unique_stops) {
        Point p = projector(stop->coordinates);

        Circle circle;
        circle.SetCenter(p).SetRadius(settings_.stop_radius).SetFillColor(std::string("white"));
        doc.Add(std::move(circle));
    }
}

void MapRenderer::RenderStopLabels(svg::Document& doc, const std::vector<const Stop*>& unique_stops,
                                   const SphereProjector& projector) const {
    for (const Stop* stop : unique_stops) {
        Point p = projector(stop->coordinates);

        AddLabelPair(doc, p, stop->name, settings_.stop_label_font_size, settings_.stop_label_offset,
                     settings_.underlayer_color, settings_.underlayer_width, std::string("black"), false);
    }
}

svg::Document MapRenderer::Render() const {
    PreparedData data = PrepareData();

    if (data.unique_stops.empty()) {
        return svg::Document{};
    }

    SphereProjector projector = CreateProjector(data.unique_stops);
    svg::Document doc;

    RenderBusLines(doc, data.sorted_bus, projector);
    RenderBusLabels(doc, data.sorted_bus, projector);
    RenderStopCircles(doc, data.unique_stops, projector);
    RenderStopLabels(doc, data.unique_stops, projector);
    return doc;
}
}  // namespace transport
