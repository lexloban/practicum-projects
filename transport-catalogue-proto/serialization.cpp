#include "serialization.h"

void SerializationCatalogue(transport::TransportCatalogue& catalogue, catalogue_serialize::Lists& lists) {
    catalogue_serialize::StopsList* proto_stops_list = lists.mutable_stop_list();

    const auto stops_count = catalogue.GetStopPtrs().size();
    for (int i = 0; i < stops_count; ++i) {
        const auto& stop_ptr = catalogue.GetStopPtr(catalogue.GetStopFromIndex(i));

        auto* proto_stop = proto_stops_list->add_stop();
        proto_stop->set_name(stop_ptr->name);

        auto* proto_coordinates = proto_stop->mutable_coordinates();
        proto_coordinates->set_lat(stop_ptr->coordinates.lat);
        proto_coordinates->set_lng(stop_ptr->coordinates.lng);

        for (const auto& [stop_to, dist] : catalogue.GetDistancesFromStop(stop_ptr->name)) {
            if (dist != 0.0) {
                auto* proto_distance = proto_stop->add_distance();
                proto_distance->set_stop_name_to(stop_to.data());
                proto_distance->set_distance(dist);
            }
        }
    }

    catalogue_serialize::BusesList* proto_buses_list = lists.mutable_bus_list();
    const auto& bus_ptrs = catalogue.GetBusPtrs();

    for (const auto& [name, bus] : bus_ptrs) {
        auto* proto_bus = proto_buses_list->add_bus();
        proto_bus->set_name(bus->name);
        proto_bus->set_is_roundtrip(bus->is_roundtrip);

        for (const auto& stop : bus->stops) {
            auto* proto_stop = proto_bus->add_stop();
            proto_stop->set_name(stop->name);
        }
    }
}
transport::TransportCatalogue DeserializationCatalogue(catalogue_serialize::StopsList* stops_list,
                                                       catalogue_serialize::BusesList* buses_list) {
    transport::TransportCatalogue catalogue;
    std::vector<transport::StopDistances> stops_distances;

    for (const auto& proto_stop : stops_list->stop()) {

// ----------------------- Stop -----------------------

        transport::Stop stop;
        stop.name = proto_stop.name();
        stop.coordinates.lat = proto_stop.coordinates().lat();
        stop.coordinates.lng = proto_stop.coordinates().lng();

        catalogue.AddStop(stop);

// ----------------------- Distances -----------------------

        transport::StopDistances distances;
        distances.name_from = stop.name;
        for (const auto& proto_distance : proto_stop.distance()) {
            distances.distances_to.push_back({proto_distance.stop_name_to(),
                                              proto_distance.distance()});
        }
        stops_distances.push_back(distances);
    }

    for (auto& dist : stops_distances) {
        catalogue.AddDistance(dist);
    }

// ----------------------- Bus -----------------------

    for (const auto& proto_bus : buses_list->bus()) {
        transport::Bus bus;
        bus.name = proto_bus.name();
        bus.is_roundtrip = proto_bus.is_roundtrip();

        const auto& stops_ptrs = catalogue.GetStopPtrs();
        for (const auto& proto_stop : proto_bus.stop()) {
            bus.stops.push_back(stops_ptrs.at(proto_stop.name()));
        }
        catalogue.AddBus(bus);
    }



    return std::move(catalogue);
}

void SerializationRenderSettings(RenderSettings settings, catalogue_serialize::Lists& lists) {
    render_serialize::RenderSettings* settings_serialize = lists.mutable_render_settings();
    
    settings_serialize->set_width(settings.width);
    settings_serialize->set_height(settings.height);
    settings_serialize->set_padding(settings.padding);
    
    settings_serialize->set_line_width(settings.line_width);
    settings_serialize->set_stop_radius(settings.stop_radius);
    
    settings_serialize->set_bus_label_font_size(settings.bus_label_font_size);
    for (const auto& bus_offset : settings.bus_label_offset) {
        settings_serialize->add_bus_label_offset(bus_offset);
    }
    
    settings_serialize->set_stop_label_font_size(settings.stop_label_font_size);
    for (const auto& stop_offset : settings.stop_label_offset) {
        settings_serialize->add_stop_label_offset(stop_offset);
    }

    auto* proto_color = settings_serialize->mutable_underlayer_color();

    if (std::holds_alternative<std::string>(settings.underlayer_color)) {

        proto_color->set_color_name(std::get<std::string>(settings.underlayer_color));
    } else if (std::holds_alternative<svg::Rgb>(settings.underlayer_color)) {
        auto* proto_rgb = proto_color->mutable_rgb();
        proto_rgb->set_red(std::get<svg::Rgb>(settings.underlayer_color).red);
        proto_rgb->set_green(std::get<svg::Rgb>(settings.underlayer_color).green);
        proto_rgb->set_blue(std::get<svg::Rgb>(settings.underlayer_color).blue);
    } else if (std::holds_alternative<svg::Rgba>(settings.underlayer_color)) {
        auto* proto_rgba = proto_color->mutable_rgba();
        proto_rgba->set_red(std::get<svg::Rgba>(settings.underlayer_color).red);
        proto_rgba->set_green(std::get<svg::Rgba>(settings.underlayer_color).green);
        proto_rgba->set_blue(std::get<svg::Rgba>(settings.underlayer_color).blue);
        proto_rgba->set_opacity(std::get<svg::Rgba>(settings.underlayer_color).opacity);
    }

    settings_serialize->set_underlayer_width(settings.underlayer_width);

    for (const auto& color : settings.color_palette) {
        auto* proto_color2 = settings_serialize->add_color_palette();

        if (std::holds_alternative<std::string>(color)) {
            proto_color2->set_color_name(std::get<std::string>(color));
        } else if (std::holds_alternative<svg::Rgb>(color)) {
            auto* proto_rgb = proto_color2->mutable_rgb();
            proto_rgb->set_red(std::get<svg::Rgb>(color).red);
            proto_rgb->set_green(std::get<svg::Rgb>(color).green);
            proto_rgb->set_blue(std::get<svg::Rgb>(color).blue);
        } else if (std::holds_alternative<svg::Rgba>(color)) {
            auto* proto_rgba = proto_color2->mutable_rgba();
            proto_rgba->set_red(std::get<svg::Rgba>(color).red);
            proto_rgba->set_green(std::get<svg::Rgba>(color).green);
            proto_rgba->set_blue(std::get<svg::Rgba>(color).blue);
            proto_rgba->set_opacity(std::get<svg::Rgba>(color).opacity);
        }
    }
}
RenderSettings DeserializationRenderSettings(render_serialize::RenderSettings* settings_serialize) {
    RenderSettings settings;

    settings.width = settings_serialize->width();
    settings.height = settings_serialize->height();
    settings.padding = settings_serialize->padding();

    settings.line_width = settings_serialize->line_width();
    settings.stop_radius = settings_serialize->stop_radius();

    settings.bus_label_font_size = settings_serialize->bus_label_font_size();
    for (const auto& bus_offset : settings_serialize->bus_label_offset()) {
        settings.bus_label_offset.push_back(bus_offset);
    }

    settings.stop_label_font_size = settings_serialize->stop_label_font_size();
    for (const auto& stop_offset : settings_serialize->stop_label_offset()) {
        settings.stop_label_offset.push_back(stop_offset);
    }

    auto* proto_underlayer_color = settings_serialize->mutable_underlayer_color();
    switch (proto_underlayer_color->type_case()) {
        case (svg_serialize::Color::kColorName): { settings.underlayer_color = proto_underlayer_color->color_name(); break; }

        case (svg_serialize::Color::kRgb): { settings.underlayer_color = svg::Rgb{static_cast<uint8_t>(proto_underlayer_color->rgb().red()),
                                                                                static_cast<uint8_t>(proto_underlayer_color->rgb().green()),
                                                                                static_cast<uint8_t>(proto_underlayer_color->rgb().blue())}; break; }

        case (svg_serialize::Color::kRgba): { settings.underlayer_color = svg::Rgba{static_cast<uint8_t>(proto_underlayer_color->rgba().red()),
                                                                                  static_cast<uint8_t>(proto_underlayer_color->rgba().green()),
                                                                                  static_cast<uint8_t>(proto_underlayer_color->rgba().blue()),
                                                                                  proto_underlayer_color->rgba().opacity()}; break; }
    }

    settings.underlayer_width = settings_serialize->underlayer_width();
    for (const auto& proto_color : settings_serialize->color_palette()) {
        switch (proto_color.type_case()) {
            case (svg_serialize::Color::kColorName): { settings.color_palette.push_back(proto_color.color_name()); break; }
            case (svg_serialize::Color::kRgb): { settings.color_palette.push_back(svg::Rgb{static_cast<uint8_t>(proto_color.rgb().red()),
                                                                                         static_cast<uint8_t>(proto_color.rgb().green()),
                                                                                         static_cast<uint8_t>(proto_color.rgb().blue())}); break; }
            case (svg_serialize::Color::kRgba): { settings.color_palette.push_back(svg::Rgba{static_cast<uint8_t>(proto_color.rgba().red()),
                                                                                           static_cast<uint8_t>(proto_color.rgba().green()),
                                                                                           static_cast<uint8_t>(proto_color.rgba().blue()),
                                                                                           proto_color.rgba().opacity()}); break; }
        }
    }

    return settings;
}


void SerializationGraph(transport::TransportGraph& graph, catalogue_serialize::Lists& lists) {
    router_serialize::EdgesMap* edges_serialize = lists.mutable_edges_map();

    edges_serialize->set_vetrex_count(graph.GetVertexCount());
    for (const auto& [edge_id, edge_info] : graph.GetEdgesMap()) {
        auto* proto_edge = edges_serialize->add_edge();

        proto_edge->set_edge_id(edge_id);

        auto* proto_edge_info = proto_edge->mutable_edge_info();

        proto_edge_info->set_from(edge_info.from.data());
        proto_edge_info->set_to(edge_info.to.data());
        proto_edge_info->set_bus_name(edge_info.bus_name.data());
        proto_edge_info->set_time(edge_info.time);
        proto_edge_info->set_span_count(edge_info.span_count);
        proto_edge_info->set_weight(edge_info.weight);
        proto_edge_info->set_vertex_id_from(edge_info.index_from);
        proto_edge_info->set_vertex_id_to(edge_info.index_to);
    }

    router_serialize::RoutingSettings* settings_serialize = lists.mutable_routing_settings();
    auto& settings = graph.GetRoutingSettings();

    settings_serialize->set_bus_wait_time(settings.bus_wait_time);
    settings_serialize->set_bus_velocity(settings.bus_velocity);
}
transport::TransportGraph DeserializationGraph(catalogue_serialize::Lists& lists) {
    router_serialize::EdgesMap* edges_serialize = lists.mutable_edges_map();

    router_serialize::RoutingSettings* settings_serialize = lists.mutable_routing_settings();
    transport::RoutingSettings settings;

    settings.bus_wait_time = settings_serialize->bus_wait_time();
    settings.bus_velocity = settings_serialize->bus_velocity();
    int vertex_count = edges_serialize->vetrex_count();

    transport::TransportGraph graph(settings, vertex_count);

    int id = 0;
    for (auto& proto_edge : edges_serialize->edge()) {
        graph::EdgeId edge_id = proto_edge.edge_id();
        graph::EdgeInfo<double> edge_info;

        edge_info.from = proto_edge.edge_info().from();
        edge_info.to = proto_edge.edge_info().to();
        edge_info.bus_name = proto_edge.edge_info().bus_name();
        edge_info.time = proto_edge.edge_info().time();
        edge_info.span_count = proto_edge.edge_info().span_count();
        edge_info.weight = proto_edge.edge_info().weight();
        edge_info.index_from = proto_edge.edge_info().vertex_id_from();
        edge_info.index_to = proto_edge.edge_info().vertex_id_to();

        id = graph.AddEdge(graph::Edge<double>{static_cast<size_t>(edge_info.index_from)
                , static_cast<size_t>(edge_info.index_to)
                , edge_info.weight});

        graph.AddEdgeInfo(id, edge_info);

    }

    return std::move(graph);
}