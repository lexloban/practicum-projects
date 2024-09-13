#include "process_queries.h"



std::vector<std::vector<Document>> ProcessQueries(
        const SearchServer& search_server,
        const std::vector<std::string>& queries) {

    std::vector<std::vector<Document>> result(queries.size());

    std::transform(std::execution::par, queries.begin(), queries.end(),
                   result.begin(),
                   [&search_server](std::string str){ return search_server.FindTopDocuments(str); });

    return result;
}

std::vector<Document> ProcessQueriesJoined(
        const SearchServer& search_server,
        const std::vector<std::string>& queries) {

    std::vector<Document> result;

    for (std::vector<Document> v : ProcessQueries(search_server, queries)) {
        for (auto doc : v) {
            result.push_back(doc);
        }
    }
    return result;
}