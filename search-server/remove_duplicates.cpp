#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {

    std::set<int> ids_trash;

    std::set<std::set<std::string>> words_key;

    for (const int document_id : search_server) {
        auto words_freqs = search_server.GetWordFrequencies(document_id);
        std::set<std::string> words;

        std::transform(words_freqs.cbegin(), words_freqs.cend(),
                       std::inserter(words, words.begin()),
                       [](const std::pair<std::string, double>& key_value)
                       { return key_value.first; });

        if (words_key.count(words)) {
            ids_trash.insert(document_id);
        }
        words_key.insert(words);
    }

    for (auto& id : ids_trash) {
        std::cout << "Found duplicate document id "s << id << std::endl;
        search_server.RemoveDocument(id);
    }

}
