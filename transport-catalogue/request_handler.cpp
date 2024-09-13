#include "request_handler.h"

RequestHandler::RequestHandler(transport::TransportCatalogue& db)
        : db_(db) {}

const std::unordered_map<std::string_view, transport::Bus*>& RequestHandler::GetRouts() const {
    return db_.GetBusPtrs();
}
const std::unordered_map<std::string_view, transport::Stop*>& RequestHandler::GetStops() const {
    return db_.GetStopPtrs();
}