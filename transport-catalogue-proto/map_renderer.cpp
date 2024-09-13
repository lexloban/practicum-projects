#include "map_renderer.h"

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

/*MapRenderer::MapRenderer(RequestHandler& handler)
        : handeler_(handler) {}*/

void MapRenderer::SetRenderSettings(const RenderSettings& settings) {
    render_settings_ = settings;
}

svg::Polyline MapRenderer::CreatePolylineShapes(const std::string& bus_name, const int color_palette_pos) {
    svg::Polyline polyline;
    (void)bus_name;
    polyline
            .SetFillColor("none")
            .SetStrokeColor(render_settings_.color_palette[color_palette_pos])
            .SetStrokeWidth(render_settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return polyline;
}
TextBusName MapRenderer::CreateTextBusNameShapes(const std::string& bus_name, const int color_palette_pos) {
    TextBusName result;

    result.front.SetFillColor(render_settings_.color_palette[color_palette_pos]);
    result.front.SetOffset({render_settings_.bus_label_offset[0],
                            render_settings_.bus_label_offset[1]});
    result.front.SetFontSize(render_settings_.bus_label_font_size);
    result.front.SetFontFamily("Verdana");
    result.front.SetFontWeight("bold");
    result.front.SetData(bus_name);


    result.back.SetOffset({render_settings_.bus_label_offset[0],
                           render_settings_.bus_label_offset[1]});
    result.back.SetFontSize(render_settings_.bus_label_font_size);
    result.back.SetFontFamily("Verdana");
    result.back.SetFontWeight("bold");
    result.back.SetData(bus_name);
    result.back.SetFillColor(render_settings_.underlayer_color);
    result.back.SetStrokeColor(render_settings_.underlayer_color);
    result.back.SetStrokeWidth(render_settings_.underlayer_width);
    result.back.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    result.back.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return result;
}
TextStopName MapRenderer::CreateTextStopNameShapes(const std::string stop_name, svg::Point point) {
    TextStopName result;

    result.front.SetPosition(point);
    result.front.SetFillColor("black");
    result.front.SetOffset({render_settings_.stop_label_offset[0],
                            render_settings_.stop_label_offset[1]});
    result.front.SetFontSize(render_settings_.stop_label_font_size);
    result.front.SetFontFamily("Verdana");
    result.front.SetData(stop_name);



    result.back.SetPosition(point);
    result.back.SetFillColor(render_settings_.underlayer_color);
    result.back.SetStrokeColor(render_settings_.underlayer_color);
    result.back.SetStrokeWidth(render_settings_.underlayer_width);
    result.back.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    result.back.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    result.back.SetOffset({render_settings_.stop_label_offset[0],
                           render_settings_.stop_label_offset[1]});
    result.back.SetFontSize(render_settings_.stop_label_font_size);
    result.back.SetFontFamily("Verdana");
    result.back.SetData(stop_name);

    return result;
}
svg::Circle MapRenderer::CreateCirclePointsShapes(svg::Point point) {
    svg::Circle circle;

    circle.SetCenter(point);
    circle.SetRadius(render_settings_.stop_radius);
    circle.SetFillColor("white");

    return circle;
}


void MapRenderer::SetCanvas(const std::unordered_map<std::string_view, transport::Bus*>& routs) {
//    const auto& routs = handeler_.GetRouts();
    std::set<std::string_view> bus_name_sort;
    std::vector<geo::Coordinates> all_coordinates;

    for (const auto& [name, bus] : routs) {
        if (!bus->stops.empty()) {
            for (const auto& stop : bus->stops) {
                if (std::find(all_coordinates.begin(), all_coordinates.end(), stop->coordinates) == all_coordinates.end()) {
                    all_coordinates.push_back({stop->coordinates.lat, stop->coordinates.lng});
                }
            }
            bus_name_sort.insert(name);
        }
    }

    const SphereProjector proj{
            all_coordinates.begin(), all_coordinates.end(),
            render_settings_.width,
            render_settings_.height,
            render_settings_.padding
    };

    int color_palete_count = render_settings_.color_palette.size() - 1;
    int color_palette_pos = 0;



    std::vector<TextBusName> buses_text_name;
    std::map<std::string, svg::Circle> circle_points;
    std::map<std::string, TextStopName> stops_text_name;
    //std::vector<std::pair<std::string, svg::Circle>> circle_points;

//////////////////////////////////////////////////////////////////////////////////////
    for (const auto& bus_name : bus_name_sort) {
        svg::Polyline polyline = CreatePolylineShapes(routs.at(bus_name)->name, color_palette_pos);
        TextBusName text_bus_name = CreateTextBusNameShapes(routs.at(bus_name)->name, color_palette_pos);
        std::optional<TextBusName> second_text_bus_name;

        bool is_first = true;
        for (const auto& stop : routs.at(bus_name)->stops) {
            const svg::Point screen_coord = proj(stop->coordinates);
            polyline.AddPoint(screen_coord);


            if (circle_points.find(stop->name) == circle_points.end()) {
                circle_points.insert({stop->name, CreateCirclePointsShapes(screen_coord)});
            }
            if (stops_text_name.find(stop->name) == stops_text_name.end()) {
                stops_text_name.insert({stop->name, CreateTextStopNameShapes(stop->name, screen_coord)});
            }

            if (is_first) {
                text_bus_name.front.SetPosition(screen_coord);
                text_bus_name.back.SetPosition(screen_coord);
                is_first = false;
            }
        }

        if (routs.at(bus_name)->is_roundtrip == false) {
            for (int i = routs.at(bus_name)->stops.size() - 2; i >= 0; i--) {
                const svg::Point screen_coord = proj(routs.at(bus_name)->stops[i]->coordinates);
                polyline.AddPoint(screen_coord);
            }
            if (routs.at(bus_name)->stops.size() > 1 &&
                routs.at(bus_name)->stops[0]->name != routs.at(bus_name)->stops[routs.at(bus_name)->stops.size() - 1]->name) {
                second_text_bus_name = CreateTextBusNameShapes(routs.at(bus_name)->name, color_palette_pos);
                second_text_bus_name->front.SetPosition(proj(routs.at(bus_name)->stops[routs.at(bus_name)->stops.size() - 1]->coordinates));
                second_text_bus_name->back.SetPosition(proj(routs.at(bus_name)->stops[routs.at(bus_name)->stops.size() - 1]->coordinates));
            }
        }

        ++color_palette_pos;
        if (color_palette_pos > color_palete_count) {
            color_palette_pos = 0;
        }

        buses_text_name.push_back(text_bus_name);
        if (second_text_bus_name.has_value()) {
            buses_text_name.push_back(second_text_bus_name.value());
        }
        document_.Add(polyline);
    }
    for (const auto& text : buses_text_name) {
        document_.Add(text.back);
        document_.Add(text.front);
    }
    for (const auto& [stop_name, circle] : circle_points) {
        document_.Add(circle);
    }
    for (const auto& [stop_name, text] : stops_text_name) {
        document_.Add(text.back);
        document_.Add(text.front);
    }
}

void MapRenderer::DrawRoutes(std::ostream& out) {
    document_.Render(out);
}