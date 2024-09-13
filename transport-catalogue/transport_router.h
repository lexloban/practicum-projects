#pragma once

#include "router.h"
#include "graph.h"
#include "transport_catalogue.h"

namespace transport {
    class TransportGraph : public graph::DirectedWeightedGraph<double> {
    public:
        TransportGraph(const transport::TransportCatalogue& catalogue
                       , const transport::RoutingSettings& routing_settings
                       , int vertex_count);

        void CreateGraph(const transport::TransportCatalogue& catalogue);

        const graph::EdgeInfo& GetEdgeInfoByIndex(const graph::EdgeId id) const;

        const transport::RoutingSettings& GetRoutingSettings();

    private:
        transport::RoutingSettings routing_settings_;
        std::unordered_map<graph::EdgeId, graph::EdgeInfo> edge_id_to_info_;
    };


    class TransportRouter : public graph::Router<double> {
    public:
        TransportRouter(const transport::TransportGraph& transport_graph)
                : graph::Router<double>(transport_graph)
                , transport_graph_(transport_graph) {
        }

    private:
        const TransportGraph& transport_graph_;
    };
}

