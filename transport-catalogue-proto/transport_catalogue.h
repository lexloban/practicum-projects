#pragma once

#include "domain.h"
#include "graph.h"

#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iterator>
#include <algorithm>

namespace transport {

    class TransportCatalogue {
    public:
        explicit TransportCatalogue() = default;

        void AddStop(Stop& stop);
        void AddDistance(StopDistances& distances);
        void AddBus(Bus& bus);
        void CreateGraph();
        void SetRoutingSettings(RoutingSettings& settings);

        Bus* GetBusInfo(const std::string& bus_name);
        StopInfo GetStopInfo(const std::string& stop_name);
        const std::unordered_map<std::string_view, Stop*>& GetStopPtrs() const;
        const Stop* GetStopPtr(const std::string& stop_name) const;
        int GetIndexFromStop(const std::string& stop_name) const;
        std::string GetStopFromIndex(const int index) const;
        const graph::EdgeInfo<double>& GetEdgeInfoByIndex(const graph::EdgeId id) const;
        const std::unordered_map<std::string_view, Bus*>& GetBusPtrs() const;
        double GetDistance(Stop* stop_from, Stop* stop_to) const;
        const std::vector<std::pair<std::string_view, double>>& GetDistancesFromStop(const std::string& stop_name);
        const graph::DirectedWeightedGraph<double>& GetGraph() const;
        const RoutingSettings& GetRoutingSettings() const;

        bool FindStop(const std::string& stop_name);

        double CalculateRouteForward(const std::string& bus);
        double CalculateRouteBackward(const std::string& bus);
        double CalculateRouteActual(const std::string& bus);
        double CalculateRouteGeographic(const std::string& bus);
        double CalculateCurvature(const double& actual, const double& geographic);


    private:
        std::deque<Stop> stops_source_;
        std::unordered_map<std::string_view, Stop*> stops_ptrs_;

        std::unordered_map<std::string_view, int> stop_name_to_index_;
        std::unordered_map<int, std::string_view> index_to_stop_name_;

        std::deque<Bus> bus_source_;
        std::unordered_map<std::string_view, Bus*> bus_ptrs_;

        struct PairStopHasher {
            size_t operator() (const std::pair<Stop*, Stop*>& pair) const {
                auto hash1 = ptr_hasher(pair.first);
                auto hash2 = ptr_hasher(pair.second);
                return hash1 + hash2 * 43;
            }
            std::hash<const void*> ptr_hasher;
        };
        std::unordered_map<std::pair<Stop*, Stop*>, double, PairStopHasher> distances_;
        std::unordered_map<std::string_view, std::vector<std::pair<std::string_view, double>>> stop_to_distances_;
        std::unordered_map<Stop*, std::vector<Bus*>> stop_to_buses_;

        int CalculateUniqueStops(const std::string& bus);
        int CalculateStops(const std::string& bus);
    };

    namespace test {
        void CalculateDistance();
    }

}


