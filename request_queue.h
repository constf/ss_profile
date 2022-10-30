#pragma once

#include "document.h"
#include "search_server.h"
#include <deque>
#include <string>
#include <vector>



class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        const std::string& query;
        size_t number_docs_found = 0;
    };
    
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    
    const SearchServer& server_;
};
