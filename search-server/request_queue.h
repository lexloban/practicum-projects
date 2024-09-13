#pragma once
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    void PushDistributionRequest(const std::vector<Document>& documents);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        QueryResult(bool empty) : is_empty(empty) {}

        bool is_empty;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& search_server_;
    // возможно, здесь вам понадобится что-то ещё
};


template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {

    std::vector<Document> docs = search_server_.FindTopDocuments(raw_query, document_predicate);

    PushDistributionRequest(docs);

    return docs;
}

