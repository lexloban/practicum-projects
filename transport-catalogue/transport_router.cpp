#include "transport_router.h"

namespace transport {
    TransportGraph::TransportGraph(const transport::TransportCatalogue& catalogue
                                   , const transport::RoutingSettings& routing_settings
                                   , int vertex_count)
                                            : graph::DirectedWeightedGraph<double>(vertex_count)
                                            , routing_settings_(routing_settings) {
        CreateGraph(catalogue);
    }

    void TransportGraph::CreateGraph(const transport::TransportCatalogue& catalogue) {
        int edge_id = 0;
        for (const auto& [bus_name_key, bus_value] : catalogue.GetBusPtrs()) { // маршруты
            for (int i = 0; i < bus_value->stops.size() - 1; ++i) { // остановки

                double distance = catalogue.GetDistance(bus_value->stops[i], bus_value->stops[i + 1]);
                double back_distance = catalogue.GetDistance(bus_value->stops[i + 1], bus_value->stops[i]);
                double time = ((distance != 0.0 ? distance : back_distance)
                               / (routing_settings_.bus_velocity * 1000)) * 60;

                int index_from = catalogue.GetIndexFromStop(bus_value->stops[i]->name);
                int index_to = catalogue.GetIndexFromStop(bus_value->stops[i + 1]->name);

                edge_id = AddEdge({static_cast<size_t>(index_from)
                                          , static_cast<size_t>(index_to)
                                          , routing_settings_.bus_wait_time + time});

                edge_id_to_info_[edge_id] = graph::EdgeInfo{bus_value->stops[i]->name
                        , bus_value->stops[i + 1]->name
                        , bus_name_key
                        , time};

                if (!bus_value->is_roundtrip) { // добавляем ребро в обратную сторону если маршрут не кольцевой

                    double back_time = ((back_distance != 0.0 ? back_distance : distance)
                                        / (routing_settings_.bus_velocity * 1000)) * 60;
                    edge_id = AddEdge({static_cast<size_t>(index_to)
                                              , static_cast<size_t>(index_from)
                                              , routing_settings_.bus_wait_time + back_time});

                    edge_id_to_info_[edge_id] = graph::EdgeInfo{bus_value->stops[i + 1]->name
                            , bus_value->stops[i]->name
                            , bus_name_key
                            , back_time};

                    if (i + 1 > 1) {
                        double total_time = routing_settings_.bus_wait_time + back_time;
                        int span_count = 1;
                        for (int j = i - 1; j >= 0; --j) {
                            double index_from_back = index_to;
                            double index_to_back = catalogue.GetIndexFromStop(bus_value->stops[j]->name);

                            back_distance = catalogue.GetDistance(bus_value->stops[j + 1], bus_value->stops[j]);
                            distance = catalogue.GetDistance(bus_value->stops[j], bus_value->stops[j + 1]);
                            back_time = ((back_distance != 0.0 ? back_distance : distance)
                                         / (routing_settings_.bus_velocity * 1000)) * 60;
                            total_time += back_time;

                            edge_id = AddEdge({static_cast<size_t>(index_from_back)
                                                      , static_cast<size_t>(index_to_back)
                                                      , total_time});

                            edge_id_to_info_[edge_id] = graph::EdgeInfo{bus_value->stops[i + 1]->name
                                    , bus_value->stops[j]->name
                                    , bus_name_key
                                    , total_time - routing_settings_.bus_wait_time
                                    , ++span_count};
                        }
                    }
                }

                if (bus_value->stops.size() > 2) {
                    double total_time = routing_settings_.bus_wait_time + time;
                    int span_count = 1;

                    for (int i2 = i + 2; i2 < bus_value->stops.size(); ++i2) {
                        index_to = catalogue.GetIndexFromStop(bus_value->stops[i2]->name);

                        distance = catalogue.GetDistance(bus_value->stops[i2 - 1], bus_value->stops[i2]);
                        back_distance = catalogue.GetDistance(bus_value->stops[i2], bus_value->stops[i2 - 1]);
                        time = ((distance != 0.0 ? distance : back_distance)
                                / (routing_settings_.bus_velocity * 1000)) * 60;
                        total_time += time;

                        edge_id = AddEdge({static_cast<size_t>(index_from)
                                                  , static_cast<size_t>(index_to)
                                                  , total_time});

                        edge_id_to_info_[edge_id] = graph::EdgeInfo{bus_value->stops[i]->name
                                , bus_value->stops[i2]->name
                                , bus_name_key
                                , total_time - routing_settings_.bus_wait_time
                                , ++span_count};
                    }
                }
            }
        }
    }

    const graph::EdgeInfo& TransportGraph::GetEdgeInfoByIndex(const graph::EdgeId id) const {
        if (edge_id_to_info_.find(id) == edge_id_to_info_.end()) {
            static graph::EdgeInfo empty;
            return empty;
        }
        return edge_id_to_info_.at(id);
    }

    const transport::RoutingSettings& TransportGraph::GetRoutingSettings() {
        return routing_settings_;
    }

}
