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

        if (word_collection_checker.count(words_set) > 0){
            docs_to_remove.push_back(doc_id);
        } else {
            word_collection_checker.insert(words_set);
        }

        delete &words_map;


        /*if (std::count(docs_to_remove.begin(), docs_to_remove.end(),*iter_small) > 0) continue;

        for(auto iter_big = next(iter_small); iter_big != search_server.end(); ++iter_big){
            if (std::count(docs_to_remove.begin(), docs_to_remove.end(),*iter_big) > 0) continue;
            const std::map<std::string, double>& small_set = search_server.GetWordFrequencies(*iter_small);
            const std::map<std::string, double>& big_set = search_server.GetWordFrequencies(*iter_big);

            if (small_set.size() != big_set.size()){
                delete &small_set;
                delete &big_set;
                continue;
            }

            auto Check2Keys = [](std::pair<std::string, double> lhv, std::pair<std::string, double> rhv){
                return lhv.first == rhv.first;
            };
            if (std::equal(small_set.begin(), small_set.end(), big_set.begin(), Check2Keys)){
                docs_to_remove.push_back(*iter_big);
            }
            delete &small_set;
            delete &big_set;
        }*/
    }

    for(int doc_id : docs_to_remove){
        using namespace std;

        search_server.RemoveDocument(doc_id);
        cout << "Found duplicate document id "s << doc_id << endl;
    }
}
