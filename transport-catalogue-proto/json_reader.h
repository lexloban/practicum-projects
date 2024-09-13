#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "router.h"
#include "transport_router.h"
#include "serialization.h"

#include <istream>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <optional>

class JsonReader {
public:
    JsonReader(std::istream& input);

    void InputJson(std::istream& input);
    void OutputJson(std::ostream& out);
    void FillCatalogue(transport::TransportCatalogue& catalogue);
    RenderSettings GetRenderSettings();
    transport::RoutingSettings GetRoutingSettings();
    SerializationSettings GetSerializationSettings();

    void OutputStat(transport::TransportCatalogue& catalogue
                    , std::ostream& out
                    , MapRenderer& renderer
                    , transport::TransportRouter& transport_router
                    , transport::TransportGraph& transport_graph);

private:
    json::Document json_doc_;
};