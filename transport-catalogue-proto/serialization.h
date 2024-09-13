#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>

#include <map_renderer.pb.h>
#include "map_renderer.h"

struct SerializationSettings {
    std::string file_name;
};


void SerializationCatalogue(transport::TransportCatalogue& catalogue, catalogue_serialize::Lists& lists);
transport::TransportCatalogue DeserializationCatalogue(catalogue_serialize::StopsList* stops_list,
                                                       catalogue_serialize::BusesList* buses_list);

void SerializationRenderSettings(RenderSettings settings, catalogue_serialize::Lists& lists);
RenderSettings DeserializationRenderSettings(render_serialize::RenderSettings* settings_serialize);


void SerializationGraph(transport::TransportGraph& graph, catalogue_serialize::Lists& lists);

transport::TransportGraph DeserializationGraph(catalogue_serialize::Lists& lists);
