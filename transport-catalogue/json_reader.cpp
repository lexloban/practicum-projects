#include "json_reader.h"
#include "log_duration.h"
#include <variant>

using namespace std;

// ---------------  JsonReader  -----------------



JsonReader::JsonReader(std::istream& input)
        : json_doc_(json::Load(input)) {
}

void JsonReader::InputJson(std::istream& input) {
    json_doc_ = json::Load(input);
}
void JsonReader::OutputJson(std::ostream& out) {
    json::Print(json_doc_, out);
}
void JsonReader::FillCatalogue(transport::TransportCatalogue& catalogue) {
    vector<transport::StopDistances> stops_distances;

    for (auto& request : json_doc_.GetRoot().AsDict().at("base_requests"s).AsArray()) {
        auto& map = request.AsDict();
        if (map.at("type"s) == "Stop"s) {
            if (!map.at("road_distances"s).AsDict().empty()) {
                auto& dists = map.at("road_distances"s).AsDict();

                transport::StopDistances stop_dists;
                stop_dists.name_from = map.at("name").AsString();

                for (auto& [stop_to, dist] : dists) {
                    stop_dists.distances_to.push_back({stop_to, dist.AsInt()});
                }

                stops_distances.push_back(std::move(stop_dists));
            }
            transport::Stop stop;
            stop.name = map.at("name").AsString();
            stop.coordinates = {map.at("latitude").AsDouble(), map.at("longitude").AsDouble()};
            catalogue.AddStop(stop);
        }
    }

    for (auto& dist : stops_distances) {
        catalogue.AddDistance(dist);
    }

    for (auto& request : json_doc_.GetRoot().AsDict().at("base_requests"s).AsArray()) {
        auto& map = request.AsDict();

        if (map.at("type"s) == "Bus"s) {
            const auto& stops_ptrs = catalogue.GetStopPtrs();
            transport::Bus bus;
            bus.name = map.at("name"s).AsString();

            for (auto& stop : map.at("stops"s).AsArray()) {
                bus.stops.push_back(stops_ptrs.at(stop.AsString()));
            }

            bus.is_roundtrip = map.at("is_roundtrip"s).AsBool();

            catalogue.AddBus(bus);
        }
    }
}
void JsonReader::GetRenderSettings(MapRenderer& renderer) {
    RenderSettings settings;

    auto& map = json_doc_.GetRoot().AsDict().at("render_settings"s).AsDict();

    settings.width = map.at("width"s).AsDouble();
    settings.height = map.at("height"s).AsDouble();
    settings.padding = map.at("padding"s).AsDouble();
    settings.line_width = map.at("line_width"s).AsDouble();
    settings.stop_radius = map.at("stop_radius"s).AsDouble();

    settings.bus_label_font_size = map.at("bus_label_font_size"s).AsInt();
    for (const auto& offset : map.at("bus_label_offset"s).AsArray()) {
        settings.bus_label_offset.push_back(offset.AsDouble());
    }
    settings.stop_label_font_size = map.at("stop_label_font_size"s).AsInt();
    for (const auto& offset : map.at("stop_label_offset"s).AsArray()) {
        settings.stop_label_offset.push_back(offset.AsDouble());
    }

    if (map.at("underlayer_color"s).IsArray()) {
        int count = map.at("underlayer_color"s).AsArray().size();

        if (count == 3) {
            svg::Rgb rgb(map.at("underlayer_color"s).AsArray()[0].AsInt(),
                         map.at("underlayer_color"s).AsArray()[1].AsInt(),
                         map.at("underlayer_color"s).AsArray()[2].AsInt());
            settings.underlayer_color = rgb;
        } else {
            svg::Rgba rgba(map.at("underlayer_color"s).AsArray()[0].AsInt(),
                           map.at("underlayer_color"s).AsArray()[1].AsInt(),
                           map.at("underlayer_color"s).AsArray()[2].AsInt(),
                           map.at("underlayer_color"s).AsArray()[3].AsDouble());
            settings.underlayer_color = rgba;
        }
    } else if (map.at("underlayer_color"s).IsString()) {
        settings.underlayer_color = map.at("underlayer_color"s).AsString();
    }

    settings.underlayer_width = map.at("underlayer_width"s).AsDouble();

    for (const auto& color : map.at("color_palette"s).AsArray()) {
        if (color.IsArray()) {
            int count = color.AsArray().size();

            if (count == 3) {
                svg::Rgb rgb(color.AsArray()[0].AsInt(),
                             color.AsArray()[1].AsInt(),
                             color.AsArray()[2].AsInt());
                settings.color_palette.push_back(rgb);
            } else {
                svg::Rgba rgba(color.AsArray()[0].AsInt(),
                               color.AsArray()[1].AsInt(),
                               color.AsArray()[2].AsInt(),
                               color.AsArray()[3].AsDouble());
                settings.color_palette.push_back(rgba);
            }
        } else if (color.IsString()) {
            settings.color_palette.push_back(color.AsString());
        }
    }

    renderer.SetRenderSettings(settings);
}
transport::RoutingSettings JsonReader::GetRoutingSettings() {
    auto& map = json_doc_.GetRoot().AsDict().at("routing_settings"s).AsDict();
    return {map.at("bus_velocity"s).AsInt(), map.at("bus_wait_time"s).AsInt()};
}
void JsonReader::OutputStat(transport::TransportCatalogue& catalogue, ostream& out) {
    json::Builder builder;
    builder.StartArray();

    transport::TransportGraph transport_graph(catalogue
                                              , GetRoutingSettings()
                                              , catalogue.GetStopPtrs().size());
    transport::TransportRouter transport_router(transport_graph);

    for (const auto& query : json_doc_.GetRoot().AsDict().at("stat_requests"s).AsArray()) {
        if (query.AsDict().at("type"s).AsString() == "Stop"s) {
            const auto stop = catalogue.GetStopInfo(query.AsDict().at("name"s).AsString());
            builder.StartDict();

            if (stop.stop == nullptr) {
                builder.Key("request_id"s).Value(json::Node{query.AsDict().at("id"s).AsInt()});
                builder.Key("error_message"s).Value(json::Node{"not found"s});
            } else {

                json::Array buses;
                for (const auto& bus : stop.buses) {
                    buses.push_back(bus->name);
                }
                builder.Key("buses"s).Value(json::Node{buses});
                builder.Key("request_id"s).Value(json::Node{query.AsDict().at("id"s).AsInt()});
            }
            builder.EndDict();
        }

        if (query.AsDict().at("type"s).AsString() == "Bus"s) {
            transport::Bus* bus = catalogue.GetBusInfo(query.AsDict().at("name"s).AsString());
            builder.StartDict();

            if (bus == nullptr) {
                builder.Key("request_id").Value(json::Node{query.AsDict().at("id"s).AsInt()});
                builder.Key("error_message").Value(json::Node{"not found"s});
            } else {
                builder.Key("curvature").Value(json::Node{bus->info.curvature});
                builder.Key("request_id").Value(json::Node{query.AsDict().at("id"s).AsInt()});
                builder.Key("route_length").Value(json::Node{bus->info.route_actual});
                builder.Key("stop_count").Value(json::Node{bus->info.stops_count});
                builder.Key("unique_stop_count").Value(json::Node{bus->info.unique_stops});
            }
            builder.EndDict();
            //arr.push_back(answer);
        }

        if (query.AsDict().at("type").AsString() == "Route"s) {
            builder.StartDict();

            int index_from = catalogue.GetIndexFromStop(query.AsDict().at("from"s).AsString());
            int index_to = catalogue.GetIndexFromStop(query.AsDict().at("to"s).AsString());

            std::optional<typename graph::Router<double>::RouteInfo> route_info;

            route_info = transport_router.BuildRoute(index_from, index_to);

            if (route_info.has_value()) {
                auto& routing_settings = transport_graph.GetRoutingSettings();

                builder.Key("items").StartArray();
                for (const auto& edge : route_info.value().edges) {
                    auto edge_info = transport_graph.GetEdgeInfoByIndex(edge);

                    {
                        builder.StartDict();

                        builder.Key("stop_name").Value(string(edge_info.from));
                        builder.Key("time").Value(routing_settings.bus_wait_time);
                        builder.Key("type").Value("Wait");

                        builder.EndDict();
                    }

                    {
                        builder.StartDict();

                        builder.Key("bus").Value((string(edge_info.bus_name)));
                        builder.Key("span_count").Value(edge_info.span_count);
                        builder.Key("time").Value(edge_info.time);
                        builder.Key("type").Value("Bus");

                        builder.EndDict();
                    }
                }
                builder.EndArray();
                builder.Key("total_time").Value(route_info->weight);

            } else {
                builder.Key("error_message").Value("not found");
            }

            builder.Key("request_id").Value(json::Node{query.AsDict().at("id"s).AsInt()});

            builder.EndDict();
        }


        if (query.AsDict().at("type"s).AsString() == "Map"s) {
            //json::Dict answer;

            std::ostringstream output;

            RequestHandler handler(catalogue);
            MapRenderer renderer(handler);
            GetRenderSettings(renderer);
            renderer.SetCanvas();
            renderer.DrawRoutes(output);

            std::string result;
            for (const auto& c : output.str()) {
                if (c == '"') {
                    result += '\"';
                } else {
                    result +=c;
                }
            }
            builder.StartDict()
                    .Key("map"s).Value(json::Node{result})
                    .Key("request_id"s).Value(json::Node{query.AsDict().at("id"s).AsInt()});
            builder.EndDict();
        }
    }
    builder.EndArray();

    json::Print(json::Document{builder.Build()}, out);
}
