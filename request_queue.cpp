#include "request_queue.h"

#include <algorithm>


RequestQueue::RequestQueue(const SearchServer& search_server): server_(search_server)  {

}


template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    std::vector<Document> found_docs = server_.FindTopDocuments(raw_query, document_predicate);
    QueryResult qr = {raw_query, found_docs.size()};
    requests_.push_back(qr);

    while(requests_.size() > min_in_day_) {
        requests_.pop_front();
    }

    return found_docs;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    // напишите реализацию
    return AddFindRequest(raw_query, [status](int, DocumentStatus doc_status, int){
        return status == doc_status;
    });
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    // напишите реализацию
    return AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}

int RequestQueue::GetNoResultRequests() const {
    return std::count_if(requests_.cbegin(), requests_.cend(), [](const QueryResult& qr){
        return qr.number_docs_found == 0;
    });
}
