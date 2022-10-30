#include <iostream>
#include "search_server.h"
#include "request_queue.h"
#include "paginator.h"
#include "remove_duplicates.h"
#include "test_example_functions.h"
#include "log_duration.h"

using namespace std::literals::string_literals;
using namespace std;


// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
	RUN_TEST(TestAddingDocumentsAndQuery);
	RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
	RUN_TEST(TestExcludeMinusWordsFromSearchResults);
	RUN_TEST(TestDocumentMatching);
	RUN_TEST(TestSortingOfFoundDocumentsWithRelevance);
	RUN_TEST(TestDocumentsRatingCalculation);
	RUN_TEST(TestRelevanceCalculationOfFoundDocs);
	RUN_TEST(TestFilteringWithUserPredicate);
	RUN_TEST(TestFilteringWithStatus);
}


// --------- Окончание модульных тестов поисковой системы -----------



int main() {
    system("chcp 65001"); // Корректное отображение Кириллицы в CLion, Windows 10

    TestSearchServer();
    
    SearchServer search_server("and in at"s);
    RequestQueue request_queue(search_server);

    search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, "big cat fancy curly collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "big dog curly sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});

    MatchDocuments(search_server, "curly -cat");
    FindTopDocuments(search_server, "curly -dog");


    // 1439 запросов с нулевым результатом
    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request"s);
    }
    // все еще 1439 запросов с нулевым результатом
    request_queue.AddFindRequest("curly dog"s);
    // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
    request_queue.AddFindRequest("big collar"s);
    // первый запрос удален, 1437 запросов с нулевым результатом
    request_queue.AddFindRequest("sparrow"s);
    std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;


    // ***************************************************************************************
    SearchServer search_server_new("and with"s);

    AddDocument(search_server_new, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {7, 2, 7});
    AddDocument(search_server_new, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});

    // дубликат документа 2, будет удалён
    AddDocument(search_server_new, 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});

    // отличие только в стоп-словах, считаем дубликатом
    AddDocument(search_server_new, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, {1, 2});

    // множество слов такое же, считаем дубликатом документа 1
    AddDocument(search_server_new, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, {1, 2});

    // добавились новые слова, дубликатом не является
    AddDocument(search_server_new, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, {1, 2});

    // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
    AddDocument(search_server_new, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, {1, 2});

    // есть не все слова, не является дубликатом
    AddDocument(search_server_new, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, {1, 2});

    // слова из разных документов, не является дубликатом
    AddDocument(search_server_new, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, {1, 2});


    cout << "Before duplicates removed: "s << search_server_new.GetDocumentCount() << endl;
    RemoveDuplicates(search_server_new);
    cout << "After duplicates removed: "s << search_server_new.GetDocumentCount() << endl;

    return 0;
}
