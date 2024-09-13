#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace transport {
    const auto EPSILON = 1e-6;

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
        bool operator==(const Stop& other) {
            return name == other.name && coordinates == other.coordinates;
        }
    };

    struct StopDistances {
        std::string name_from;
        std::vector<std::pair<std::string, int64_t>> distances_to;
        bool operator==(const StopDistances& other) {
            return name_from == other.name_from && distances_to == other.distances_to;
        }
    };

    struct BusInfo {
        int stops_count = 0;
        int unique_stops = 0;
        double route_actual = 0.0;
        double route_geographic = 0.0;
        double curvature = 0.0;
    };

    struct Bus {
        std::string name;
        std::vector<Stop*> stops;
        bool is_roundtrip;
        BusInfo info;
    };

    struct StopInfo {
        Stop* stop = {};
        std::vector<transport::Bus*> buses = {};
    };

    struct RoutingSettings {
        int bus_velocity = 0;
        int bus_wait_time = 0;
    };
}


/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */