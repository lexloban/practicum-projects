#include "search_server.h"

using namespace std;

SearchServer::SearchServer(const string& stop_words_text) : SearchServer(string_view(stop_words_text)) {}

SearchServer::SearchServer(string_view stop_words_text) : SearchServer(SplitIntoWords(stop_words_text)) {}

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status,
                               const vector<int>& ratings) {

    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw invalid_argument("Invalid document_id"s);
    }

    docs_source_[document_id] = document;

    string_view doc = docs_source_.at(document_id);

    const auto words = SplitIntoWordsNoStop(doc);

    const double inv_word_count = 1.0 / words.size();
    for (auto word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        document_to_word_freqs_[document_id][word] += inv_word_count;
        words_in_docs_[document_id].insert(word);
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.insert(document_id);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

/*int SearchServer::GetDocumentId(int index) const {
    return document_ids_.at(index);
}*/

const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) {
    if (!document_to_word_freqs_.count(document_id)) {
        const static map<std::string_view, double> empty_map;
        return empty_map;
    }
    return document_to_word_freqs_.at(document_id);
}

const set<string_view> SearchServer::GetWordsInDocs(int document_id) {
    if (words_in_docs_.count(document_id) == 0) {
        return set<string_view>();
    }
    return words_in_docs_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) {
    for (auto doc : document_to_word_freqs_.at(document_id)) {
        if (word_to_document_freqs_.at(doc.first).empty()) {
            word_to_document_freqs_.erase(doc.first);
        }
        word_to_document_freqs_.at(doc.first).erase(document_id);
    }
    documents_.erase(document_id);
    document_ids_.erase(document_id);
    document_to_word_freqs_.erase(document_id);
}

void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id) {
    RemoveDocument(document_id);
}

void SearchServer::RemoveDocument(const execution::parallel_policy&, int document_id) {

    vector<const string_view*> v(document_to_word_freqs_.at(document_id).size());

    transform(execution::par,
              document_to_word_freqs_.at(document_id).begin(), document_to_word_freqs_.at(document_id).end(),
              v.begin(),
              [](const auto& doc) {
                    return &doc.first;
    } );

    std::for_each(std::execution::par,
                  v.begin(), v.end(),
                  [this, &document_id](auto word) {
        if (word_to_document_freqs_.at(*word).empty()) {
           word_to_document_freqs_.erase(*word);
        }
        word_to_document_freqs_.at(*word).erase(document_id);
    });

    v.erase(v.begin(), v.end());
    documents_.erase(document_id);
    document_ids_.erase(document_id);
    document_to_word_freqs_.erase(document_id);
}

set<int>::iterator SearchServer::begin() const {
    return document_ids_.begin();
}

std::set<int>::iterator SearchServer::end() const {
    return document_ids_.end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SearchServer::MATCHED_WORDS_IN_DOC SearchServer::MatchDocument(const std::string& raw_query,
                                                                                 int document_id) const {
    return MatchDocument(execution::seq, raw_query, document_id);
}

SearchServer::MATCHED_WORDS_IN_DOC SearchServer::MatchDocument(const std::execution::sequenced_policy&,
                                                                                 const std::string& raw_query,
                                                                                 int document_id) const {
    const auto query = ParseQuery(raw_query);

    std::vector<std::string_view> matched_words;

    bool is_minus_word = any_of(execution::seq,
                                query.minus_words.begin(), query.minus_words.end(),
                                [this, &document_id] (const auto& word) {
                                    return document_to_word_freqs_.count(document_id)
                                    && document_to_word_freqs_.at(document_id).count(word);
                                });

    if (is_minus_word) {
        return { matched_words, documents_.at(document_id).status };
    }

    matched_words.resize(query.plus_words.size());

    std::copy_if(std::execution::seq,
                 query.plus_words.begin(), query.plus_words.end(),
                 matched_words.begin(),
                 [this, &document_id](const auto& word) {
                     return document_to_word_freqs_.count(document_id) && document_to_word_freqs_.at(document_id).count(word);
                 });

    auto pos = std::find(std::execution::seq, matched_words.begin(), matched_words.end(), "");
    std::sort(std::execution::seq, matched_words.begin(), pos);
    matched_words.erase(std::unique(std::execution::seq, matched_words.begin(), pos), matched_words.end());

    return { matched_words, documents_.at(document_id).status };

}

SearchServer::MATCHED_WORDS_IN_DOC SearchServer::MatchDocument(const execution::parallel_policy&,
                                                                                 string_view raw_query,
                                                                                 int document_id) const {
    const auto query = ParseQuery(raw_query, true);

    std::vector<std::string_view> matched_words;

    bool is_minus_word = any_of(execution::par,
                                query.minus_words.begin(), query.minus_words.end(),
                                [this, &document_id] (const auto& word) {
        return document_to_word_freqs_.count(document_id) && document_to_word_freqs_.at(document_id).count(word);
    });

    if (is_minus_word) {
        return { matched_words, documents_.at(document_id).status };
    }

    matched_words.resize(query.plus_words.size());

    std::copy_if(std::execution::par,
                   query.plus_words.begin(), query.plus_words.end(),
                   matched_words.begin(),
                   [this, &document_id](const auto& word) {
                        return word_to_document_freqs_.count(word) &&
                               word_to_document_freqs_.at(word).count(document_id);
    });

    auto pos = std::find(std::execution::par, matched_words.begin(), matched_words.end(), "");
    std::sort(std::execution::par, matched_words.begin(), pos);
    matched_words.erase(std::unique(std::execution::par, matched_words.begin(), pos), matched_words.end());

    return { matched_words, documents_.at(document_id).status };

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SearchServer::IsStopWord(const string_view& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(string_view word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(string_view text) const {
    vector<string_view> words;
    for (auto word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw invalid_argument("Word "s + /*word + */" is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

double SearchServer::ComputeWordInverseDocumentFreq(string_view word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {
    if (text.empty()) {
        throw invalid_argument("Query word is empty"s);
    }
    string_view word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw invalid_argument("Query word "s + /*text + */" is invalid"s);
    }

    return { word, is_minus, IsStopWord(word) };
}

SearchServer::Query SearchServer::ParseQuery(const string_view text, bool is_par) const {
    Query result;
    auto words = SplitIntoWords(text);

    if (is_par) {
        sort(execution::par, words.begin(), words.end());
        words.erase(unique(execution::seq, words.begin(), words.end()), words.end());
    } else {
        sort(execution::seq, words.begin(), words.end());
        words.erase(unique(execution::seq, words.begin(), words.end()), words.end());
    }

    for (auto word : words) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            !query_word.is_minus ? result.plus_words.push_back(query_word.data) : result.minus_words.push_back(query_word.data);
        }
    }
    return result;
}






