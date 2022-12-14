#include "search_server.h"
#include "string_processing.h"
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include "log_duration.h"

using namespace std::literals::string_literals;


void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }
    const auto words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.insert(document_id);
}

void SearchServer::RemoveDocument(int document_id) {
    if (document_id < 0) return;
    if (documents_.count(document_id) == 0 || std::count(document_ids_.begin(), document_ids_.end(),document_id) == 0) return;

    // cut out the document's data from word to docs frequencies
    for(auto [word, wd_freq] : word_to_document_freqs_){
        auto iter = wd_freq.lower_bound(document_id);
        if (iter == wd_freq.end()) continue;
        if (iter->first > document_id) continue;
        wd_freq.erase(iter);
    }

    // erase docement's data from documents map
    auto iter = documents_.lower_bound(document_id);
    if (iter != documents_.end() && iter->first == document_id){
        documents_.erase(iter);
    }

    // erase document id from document_ids vector
    //std::remove(document_ids_.begin(), document_ids_.end(),document_id);
    document_ids_.erase(document_id);
}


std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
    });
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

const std::set<int>::const_iterator SearchServer::begin() const {
    return document_ids_.cbegin();
}

const std::set<int>::const_iterator SearchServer::end() const {
    return document_ids_.cend();
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query);

    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return {matched_words, documents_.at(document_id).status};
}


const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const {
    std::map<std::string, double> *result = new std::map<std::string, double>();

    if (std::count(document_ids_.begin(), document_ids_.end(),document_id) == 0) return *result;
    if (documents_.count(document_id) == 0) return *result;

    for(const auto [word, wd_map] : word_to_document_freqs_){
        if (wd_map.count(document_id) > 0){
            (*result)[word] = wd_map.at(document_id);
            //result.emplace(std::pair{word, wd_map.at(document_id)});
        }
    }

    return *result;
}



bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string& word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word "s + word + " is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}


SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const {
    if (text.empty()) {
        throw std::invalid_argument("Query word is empty"s);
    }
    std::string word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw std::invalid_argument("Query word "s + text + " is invalid");
    }

    return {word, is_minus, IsStopWord(word)};
}


SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query result;
    for (const std::string& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            } else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}



void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status) {
    std::cout << "{ "s
         << "document_id = "s << document_id << ", "s
         << "status = "s << static_cast<int>(status) << ", "s
         << "words ="s;
    for (const std::string& word : words) {
        std::cout << ' ' << word;
    }
    std::cout << "}"s << std::endl;
}

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
                 const std::vector<int>& ratings) {
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    } catch (const std::invalid_argument& e) {
        std::cout << "???????????? ???????????????????? ?????????????????? "s << document_id << ": "s << e.what() << std::endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query) {
    try {
        LOG_DURATION_STREAM("Operation time", std::cout);

        std::cout << "???????????????????? ???????????? ???? ??????????????: "s << raw_query << std::endl;
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    } catch (const std::invalid_argument& e) {
        std::cout << "???????????? ????????????: "s << e.what() << std::endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const std::string& query) {
    try {
        LOG_DURATION_STREAM("Operation time", std::cout);

        std::cout << "?????????????? ???????????????????? ???? ??????????????: "s << query << std::endl;

        for(auto iter = search_server.begin(); iter != search_server.end(); ++iter){
            const auto [words, status] = search_server.MatchDocument(query, *iter);
            PrintMatchDocumentResult(*iter, words, status);
        }
    } catch (const std::invalid_argument& e) {
        std::cout << "???????????? ???????????????? ???????????????????? ???? ???????????? "s << query << ": "s << e.what() << std::endl;
    }
}

