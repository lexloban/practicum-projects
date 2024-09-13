#pragma once

#include "transport_catalogue.h"

class RequestHandler {
public:
    RequestHandler(transport::TransportCatalogue& db);

    const std::unordered_map<std::string_view, transport::Bus*>& GetRouts() const;
    const std::unordered_map<std::string_view, transport::Stop*>& GetStops() const;

private:
    const transport::TransportCatalogue& db_;
};