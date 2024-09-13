#include "transport_catalogue.h"

#include <iostream>

using namespace std;

void transport::TransportCatalogue::AddStop(Stop& stop) {
    stops_source_.push_back(stop);
    stops_ptrs_.insert({stops_source_.back().name, &stops_source_.back()});
    stop_to_buses_[stops_ptrs_.at(stop.name)];

    stop_name_to_index_[stops_source_.back().name] = stops_source_.size() - 1;
    index_to_stop_name_[stops_source_.size() - 1] = stops_source_.back().name;
}
void transport::TransportCatalogue::AddDistance(StopDistances& distances) {
    auto& stop_from = stops_ptrs_.at(distances.name_from);
    stop_to_distances_[stop_from->name];
    for (const auto& [stop, metrs] : distances.distances_to) {
        auto& stop_to = stops_ptrs_.at(stop);
        distances_.insert({{stop_from, stop_to}, metrs});

        stop_to_distances_.at(stop_from->name).push_back({stop_to->name, metrs});
    }
}
void transport::TransportCatalogue::AddBus(Bus& bus) {
    bus_source_.push_back(bus);
    bus_ptrs_.insert({bus_source_.back().name, &bus_source_.back()});

    unordered_set<string_view> uniques;
    for (auto& stop : bus.stops) {
        uniques.insert(stop->name);
        stop_to_buses_[stop];
        if (find(stop_to_buses_.at(stop).begin(), stop_to_buses_.at(stop).end(), bus_ptrs_.at(bus.name)) == stop_to_buses_.at(stop).end()) {
            stop_to_buses_[stop].push_back(bus_ptrs_.at(bus.name));
        }
    }
    bus_source_.back().info.unique_stops = uniques.size();
    bus_source_.back().info.stops_count = CalculateStops(bus.name);
}

int transport::TransportCatalogue::CalculateUniqueStops(const std::string& bus) {
    unordered_set<string_view> uniques;
    for (const auto& stop : bus_ptrs_.at(bus)->stops) {
        uniques.insert(stop->name);
    }
    return static_cast<int>(uniques.size());
}
int transport::TransportCatalogue::CalculateStops(const std::string& bus) {
    int stops_count = static_cast<int>(bus_ptrs_.at(bus)->stops.size());
    if (bus_ptrs_.at(bus)->is_roundtrip == false) {
        stops_count = stops_count * 2 - 1;
    }
    return stops_count;
}
double transport::TransportCatalogue::CalculateRouteForward(const std::string& bus) {
    double distance = 0.0;
    for (auto it1 = bus_ptrs_.at(bus)->stops.begin(), it2 = it1 + 1; it2 < bus_ptrs_.at(bus)->stops.end(); ++it1, ++it2) {
        auto forward_dist = GetDistance((*it1), (*it2));
        auto backward_dist = GetDistance((*it2), (*it1));

        if (forward_dist == 0) {
            distance += backward_dist;
        } else {
            distance += forward_dist;
        }
    }
    return distance;
}
double transport::TransportCatalogue::CalculateRouteBackward(const std::string& bus) {
    double distance = 0.0;
    for (auto it1 = bus_ptrs_.at(bus)->stops.begin(), it2 = it1 + 1; it2 < bus_ptrs_.at(bus)->stops.end(); ++it1, ++it2) {
        auto backward_dist = GetDistance((*it2), (*it1));
        if (backward_dist > 0) {
            distance += backward_dist;
        } else {
            distance += GetDistance((*it1), (*it2));
        }
    }
    return distance;
}
double transport::TransportCatalogue::CalculateRouteActual(const std::string& bus) {
    double distance_forward = CalculateRouteForward(bus);

    if (bus_ptrs_.at(bus)->is_roundtrip == false) {
        double distance_backward = CalculateRouteBackward(bus);
        return distance_forward + distance_backward;
    }

    return distance_forward;
}
double transport::TransportCatalogue::CalculateRouteGeographic(const std::string& bus) {
    double distance = 0.0;
    for (auto it1 = bus_ptrs_.at(bus)->stops.begin(), it2 = it1 + 1; it2 < bus_ptrs_.at(bus)->stops.end(); ++it1, ++it2) {
        distance += ComputeDistance((*it1)->coordinates, (*it2)->coordinates);
    }

    if (bus_ptrs_.at(bus)->is_roundtrip == false) {
        distance *= 2;
    }

    return distance;
}
double transport::TransportCatalogue::CalculateCurvature(const double& actual, const double& geographic) {
    return actual / geographic;
}

bool transport::TransportCatalogue::FindStop(const string& stop_name) {
    if (stops_ptrs_.find(stop_name) == stops_ptrs_.end()) {
        return false;
    }
    return true;
}

transport::Bus* transport::TransportCatalogue::GetBusInfo(const string& bus_name) {
    if (bus_ptrs_.find(bus_name) == bus_ptrs_.end()) {
        return nullptr;
    }
    auto& bus_link = bus_ptrs_.at(bus_name);
    bus_link->info.route_actual = CalculateRouteActual(bus_name);
    bus_link->info.route_geographic = CalculateRouteGeographic(bus_name);
    bus_link->info.curvature = CalculateCurvature(bus_link->info.route_actual, bus_link->info.route_geographic);

    return bus_link;
}
transport::StopInfo transport::TransportCatalogue::GetStopInfo(const string& stop_name) {
    if (!FindStop(stop_name)) {
        StopInfo info;
        info.stop = nullptr;
        return info;
    }
    sort((stop_to_buses_.at(stops_ptrs_.at(stop_name))).begin(), (stop_to_buses_.at(stops_ptrs_.at(stop_name))).end(),
         [](Bus* left, Bus* right){ return left->name < right->name; });

    StopInfo info;
    info.stop = stops_ptrs_.at(stop_name);
    info.buses = stop_to_buses_.at(stops_ptrs_.at(stop_name));

    return info;
}

const std::unordered_map<std::string_view, transport::Stop*>& transport::TransportCatalogue::GetStopPtrs() const {
    return stops_ptrs_;
}
const transport::Stop* transport::TransportCatalogue::GetStopPtr(const std::string& stop_name) const {
    if (stops_ptrs_.find(stop_name) == stops_ptrs_.end()) {
        return nullptr;
    }
    return stops_ptrs_.at(stop_name);

}
int transport::TransportCatalogue::GetIndexFromStop(const std::string& stop_name) const {
    return stop_name_to_index_.at(stop_name);
}
std::string transport::TransportCatalogue::GetStopFromIndex(const int index) const {
    return std::string(index_to_stop_name_.at(index));
}
const std::unordered_map<std::string_view, transport::Bus*>& transport::TransportCatalogue::GetBusPtrs() const {
    return bus_ptrs_;
}
double transport::TransportCatalogue::GetDistance(Stop* stop_from, Stop* stop_to) const {
    if (distances_.find({stop_from, stop_to}) == distances_.end()) {
        return 0.0;
    }
    return distances_.at({stop_from, stop_to});
}
const std::vector<std::pair<std::string_view, double>>& transport::TransportCatalogue::GetDistancesFromStop(const std::string& stop_name) {
    if (stop_to_distances_.find(stop_name) == stop_to_distances_.end()) {
        static const std::vector<std::pair<std::string_view, double>> empty;
        return empty;
    }
    return stop_to_distances_.at(stop_name);
}
