#pragma once
#include "read_input_functions.h"
#include "string_processing.h"
#include "concurrent_map.h"


#include <iostream>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <deque>
#include <numeric>
#include <execution>

using namespace std::string_literals;


const int MAX_RESULT_DOCUMENT_COUNT = 5;

const auto EPSILON = 1e-6;

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    explicit SearchServer(const std::string& stop_words_text);

    explicit SearchServer(std::string_view stop_words_text);

    void AddDocument(int document_id, const std::string& document, DocumentStatus status,
                     const std::vector<int>& ratings);

    using MATCHED_WORDS_IN_DOC = std::tuple<std::vector<std::string_view>, DocumentStatus>;

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(std::string_view raw_query,
                                           DocumentPredicate document_predicate) const;
    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query,
                                           DocumentPredicate document_predicate) const;

    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentStatus status) const;
    template<typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query, DocumentStatus status) const;

    std::vector<Document> FindTopDocuments(std::string_view raw_query) const;
    template<typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query) const;



    int GetDocumentCount() const;

    //int GetDocumentId(int index) const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id);

    const std::set<std::string_view> GetWordsInDocs(int document_id);


    // Не понял как группировать
    void RemoveDocument(int document_id);
    void RemoveDocument(const std::execution::sequenced_policy&, int document_id);
    void RemoveDocument(const std::execution::parallel_policy&, int document_id);



    std::set<int>::iterator begin() const;

    std::set<int>::iterator end() const;

    MATCHED_WORDS_IN_DOC MatchDocument(const std::string& raw_query,
                                                                       int document_id) const;

    MATCHED_WORDS_IN_DOC MatchDocument(const std::execution::sequenced_policy&,
                                                                       const std::string& raw_query,
                                                                       int document_id) const;

    MATCHED_WORDS_IN_DOC MatchDocument(const std::execution::parallel_policy&,
                                                                       std::string_view raw_query,
                                                                       int document_id) const;

private:
    std::map<int, std::string> docs_source_;
    std::string stop_words_source_;

    const std::set<std::string, std::less<>> stop_words_;
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> document_to_word_freqs_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;
    std::map<int, std::set<std::string_view>> words_in_docs_;



    bool IsStopWord(const std::string_view& word) const;

    static bool IsValidWord(std::string_view word);

    std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string_view text) const;

    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };

    Query ParseQuery(string_view text, bool is_par = false) const;

    double ComputeWordInverseDocumentFreq(std::string_view word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query,
                                           DocumentPredicate document_predicate) const;

    template <typename ExecutionPolicy, typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const ExecutionPolicy& policy, const Query& query,
                                           DocumentPredicate document_predicate) const;

};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument("Some of stop words are invalid"s);
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
                                                     DocumentPredicate document_predicate) const {
    const auto query = ParseQuery(raw_query);

    auto matched_documents = FindAllDocuments(query, document_predicate);

    sort(matched_documents.begin(), matched_documents.end(),
         [](const Document& lhs, const Document& rhs) {
             if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
                 return lhs.rating > rhs.rating;
             }
             else {
                 return lhs.relevance > rhs.relevance;
             }
         });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query,
                                       DocumentPredicate document_predicate) const {

    Query query;

    if (is_same_v<ExecutionPolicy, execution::parallel_policy>) {
        query = ParseQuery(raw_query, true);
    } else {
        query = ParseQuery(raw_query);
    }


    auto matched_documents = FindAllDocuments(policy, query, document_predicate);

    sort(matched_documents.begin(), matched_documents.end(),
         [](const Document& lhs, const Document& rhs) {
             if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
                 return lhs.rating > rhs.rating;
             }
             else {
                 return lhs.relevance > rhs.relevance;
             }
         });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
    });
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(const ExecutionPolicy& policy, std::string_view raw_query) const {
    return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const SearchServer::Query& query,
                                                     DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;

    for (auto word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (auto word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}

template <typename ExecutionPolicy, typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const ExecutionPolicy& policy, const SearchServer::Query& query,
                                       DocumentPredicate document_predicate) const {
   ConcurrentMap<int, double> document_to_relevance(100);

    for_each(policy,
            query.plus_words.begin(), query.plus_words.end(),
             [this, &document_to_relevance, & document_predicate](auto word) {
                 if (word_to_document_freqs_.count(word) > 0) {
                     const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                     for (const auto [document_id, term_freq]: word_to_document_freqs_.at(word)) {
                         const auto &document_data = documents_.at(document_id);
                         if (document_predicate(document_id, document_data.status, document_data.rating)) {
                             document_to_relevance[document_id].ref_to_value += term_freq * inverse_document_freq;
                         }
                     }
                 }
    });

    for_each(policy,
             query.minus_words.begin(), query.minus_words.end(),
             [this, &document_to_relevance](auto word){
                 if (word_to_document_freqs_.count(word) > 0) {
                     for (const auto [document_id, _]: word_to_document_freqs_.at(word)) {
                         document_to_relevance.Erase(document_id);
                     }
                 }
    });

    std::vector<Document> matched_documents;
    for (const auto& [document_id, relevance] : document_to_relevance.BuildOrdinaryMap()) {
        matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
    }

    return matched_documents;
}
