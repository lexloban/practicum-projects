#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "router.h"
#include "transport_router.h"

#include <istream>
#include <iostream>
#include <string>
#include <sstream>
#include <optional>

class JsonReader {
private:
    transport::RoutingSettings GetRoutingSettings();
public:
    JsonReader(std::istream& input);

    void InputJson(std::istream& input);
    void OutputJson(std::ostream& out);
    void FillCatalogue(transport::TransportCatalogue& catalogue);
    void GetRenderSettings(MapRenderer& renderer);

    void OutputStat(transport::TransportCatalogue& catalogue, std::ostream& out);

private:
    json::Document json_doc_;
};