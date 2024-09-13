#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server) : search_server_(search_server) {}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {

    std::vector<Document> docs = search_server_.FindTopDocuments(raw_query, status);

    PushDistributionRequest(docs);

    return docs;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {

    std::vector<Document> docs = search_server_.FindTopDocuments(raw_query);

    PushDistributionRequest(docs);

    return docs;
}

void RequestQueue::PushDistributionRequest(const std::vector<Document>& documents) {
    if (documents.empty()) {
        requests_.push_back(QueryResult(false));
    }
    else {
        requests_.push_back(QueryResult(true));
    }

    if (requests_.size() > min_in_day_) {
        requests_.pop_front();
    }
}

int RequestQueue::GetNoResultRequests() const {
    int count = 0;
    for (auto element : requests_) {
        if (element.is_empty == false) {
            ++count;
        }
    }

    return count;
}