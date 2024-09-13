#include "transport_catalogue.h"
#include "json_reader.h"
#include "transport_router.h"

#include <iostream>
#include <fstream>
#include <string_view>


using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        transport::TransportCatalogue catalogue;
        std::ifstream data_file("1_input.json"s);
        JsonReader json_reader(data_file);
        json_reader.FillCatalogue(catalogue);

        transport::TransportGraph transport_graph(catalogue
                , json_reader.GetRoutingSettings()
                , catalogue.GetStopPtrs().size());

        const auto serialization_settings = json_reader.GetSerializationSettings();
        std::ofstream output(serialization_settings.file_name, std::ios::binary);

        catalogue_serialize::Lists lists;

        SerializationCatalogue(catalogue, lists);
        SerializationRenderSettings(json_reader.GetRenderSettings(), lists);
        SerializationGraph(transport_graph, lists);


        lists.SerializeToOstream(&output);


    } else if (mode == "process_requests"sv) {
        std::ifstream data_file("2_input.json"s);
        JsonReader json_reader(data_file);
        auto serialize_settings = json_reader.GetSerializationSettings();

        std::ifstream input(serialize_settings.file_name, std::ios::binary);

        catalogue_serialize::Lists lists;
        lists.ParseFromIstream(&input);
        catalogue_serialize::StopsList* stops_list = lists.mutable_stop_list();
        catalogue_serialize::BusesList* buses_list = lists.mutable_bus_list();
        render_serialize::RenderSettings* settings_serialize = lists.mutable_render_settings();
//        router_serialize::EdgesMap* graph_serialize = lists.mutable_edges_map();

        transport::TransportCatalogue catalogue = DeserializationCatalogue(stops_list, buses_list);
        MapRenderer renderer;
        renderer.SetRenderSettings(DeserializationRenderSettings(settings_serialize));
        transport::TransportGraph graph = DeserializationGraph(lists);
        transport::TransportRouter router(graph);

        std::ofstream output("output.json"s);
        json_reader.OutputStat(catalogue, output, renderer, router, graph);

    } else {
        PrintUsage();
        return 1;
    }
}
