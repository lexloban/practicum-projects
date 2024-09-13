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

        TransportGraph(transport::RoutingSettings routing_settings
                       , int vertex_count);

        void CreateGraph(const transport::TransportCatalogue& catalogue);

        void AddEdgeInfo(graph::EdgeId edge_id, graph::EdgeInfo<double> edge_info);

        const graph::EdgeInfo<double>& GetEdgeInfoByIndex(const graph::EdgeId id) const;

        const std::unordered_map<graph::EdgeId, graph::EdgeInfo<double>>& GetEdgesMap() const;

        const transport::RoutingSettings& GetRoutingSettings();

        int GetVertexCount() const;

    private:
        transport::RoutingSettings routing_settings_;
        std::unordered_map<graph::EdgeId, graph::EdgeInfo<double>> edge_id_to_info_;
        int vertex_count_;
    };


    class TransportRouter : public graph::Router<double> {
    public:
        TransportRouter(const transport::TransportGraph& transport_graph)
                : graph::Router<double>(transport_graph)
                , transport_graph_(transport_graph) {
        }

        /*TransportRouter(transport::TransportGraph&& transport_graph)
                : graph::Router<double>(transport_graph)
                , transport_graph_(std::move(transport_graph)) {
        }*/

    private:
        const TransportGraph& transport_graph_;
    };
}

