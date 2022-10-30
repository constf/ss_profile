#include "remove_duplicates.h"
#include <vector>
#include <iterator>
#include <iostream>

#include "search_server.h"

void RemoveDuplicates(SearchServer& search_server){
    std::vector<int> docs_to_remove;
    std::set<std::set<std::string>> word_collection_checker;

    for(const int doc_id : search_server){
        const std::map<std::string, double>& words_map = search_server.GetWordFrequencies(doc_id);
        std::set<std::string> words_set;
        for(const auto val_pair : words_map) words_set.insert(val_pair.first);

        auto res = word_collection_checker.insert(words_set);
        if (!res.second) docs_to_remove.push_back(doc_id);

        delete &words_map;
    }

    for(int doc_id : docs_to_remove){
        using namespace std;

        search_server.RemoveDocument(doc_id);
        cout << "Found duplicate document id "s << doc_id << endl;
    }
}
