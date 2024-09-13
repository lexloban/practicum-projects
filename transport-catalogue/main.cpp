#include "transport_catalogue.h"
#include "json_reader.h"
#include "log_duration.h"

#include <iostream>
#include <fstream>

using namespace std;

void RunFile() {
    using namespace transport;

    TransportCatalogue catalogue;
    ifstream data_file("input.json"s);
    JsonReader json_reader(data_file);
    json_reader.FillCatalogue(catalogue);
    ofstream output("output.json"s);
    json_reader.OutputStat(catalogue, output);
}

void RunCin() {
    using namespace transport;

    TransportCatalogue catalogue;
    JsonReader json_reader(cin);
    json_reader.FillCatalogue(catalogue);
    json_reader.OutputStat(catalogue, cout);
}

int main() {
    RunCin();
//    RunFile();

    return 0;
}
