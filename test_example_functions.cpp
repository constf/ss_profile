#include "test_example_functions.h"
#include <iostream>
#include "search_server.h"

using namespace std;



const double DOUBLES_ARE_SAME_LIMIT = 1e-6;

bool AreDoublesSame(double lhd, double rhd){
    return ( std::abs(lhd - rhd) < DOUBLES_ARE_SAME_LIMIT );
}


void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
                const std::string& hint) {
    if (!value) {
        std::cout << file << "("s << line << "): "s << func << ": "s;
        std::cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            std::cout << " Hint: "s << hint;
        }
        std::cout << std::endl;
        abort();
    }
}


void TestAddingDocumentsAndQuery(){
    const int doc_id1 = 1;
    const std::string content1 = "cat in the city of dogs with style"s;
    const std::vector<int> ratings1 = { 1, 2, 3 };

    const int doc_id2 = 2;
    const std::string content2 = "dog with magic wand persuing red fox"s;
    const std::vector<int> ratings2 = { 5, 6, 7 };

    const int doc_id3 = 3;
    const std::string content3 = "fox running on public roads with red light"s;
    const std::vector<int> ratings3 = { 3, 2, 4 };

    SearchServer server("and at"s);
    server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
    server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
    server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);

    auto found_docs1 = server.FindTopDocuments("in"s);
    ASSERT_EQUAL(found_docs1.size(), 1);

    auto found_docs2 = server.FindTopDocuments("fox"s);
    ASSERT_EQUAL(found_docs2.size(), 2);

    auto found_docs3 = server.FindTopDocuments("road"s);
    ASSERT_EQUAL(found_docs3.size(), 0);

}


void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = { 1, 2, 3 };

    const int doc_id2 = 542;
    const std::string content2 = "in garden"s;
    const std::vector<int> ratings2 = { 5, 6, 7 };

    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server(""s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);

        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id2);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server2("in the"s);
        //server2.SetStopWords("in the"s);
        server2.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server2.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        auto ftd = server2.FindTopDocuments("in"s);
        ASSERT_EQUAL(ftd.size(), 0);
    }
}


void TestExcludeMinusWordsFromSearchResults() {
    const int doc_id1 = 1;
    const std::string content1 = "cat in the city of dogs with style"s;
    const std::vector<int> ratings1 = { 1, 2, 3 };

    const int doc_id2 = 2;
    const std::string content2 = "dog with magic wand persuing red fox"s;
    const std::vector<int> ratings2 = { 5, 6, 7 };

    const int doc_id3 = 3;
    const std::string content3 = "fox running on public roads with red light"s;
    const std::vector<int> ratings3 = { 3, 2, 4 };

    {
        SearchServer server("in the with"s);;
        //server.SetStopWords("in the with");
        server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);

        const auto found_docs1 = server.FindTopDocuments("red dog"s);
        ASSERT_EQUAL(found_docs1.size(), 2);

        const auto found_docs2 = server.FindTopDocuments("red dog -light"s);
        ASSERT_EQUAL(found_docs2.size(), 1);
        ASSERT_EQUAL(found_docs2[0].id, doc_id2);
    }

}

void TestDocumentMatching() {
    const int doc_id1 = 1;
    const std::string content1 = "cat in the city of dogs with style"s;
    const std::vector<int> ratings1 = { 1, 2, 3 };

    const int doc_id2 = 2;
    const std::string content2 = "dog with magic wand persuing red fox"s;
    const std::vector<int> ratings2 = { 5, 6, 7 };

    const int doc_id3 = 3;
    const std::string content3 = "fox running on public roads with red light"s;
    const std::vector<int> ratings3 = { 3, 2, 4 };

    SearchServer server("in the with and on of"s);
    //server.SetStopWords("in the with and on of"s);
    server.AddDocument(doc_id1, content1, DocumentStatus::ACTUAL, ratings1);
    server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
    server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings3);

    const auto[mw1, status1] = server.MatchDocument("cat of style"s, doc_id1);
    ASSERT_EQUAL(mw1.size(), 2);
    ASSERT_EQUAL(mw1[0], "cat"s);
    ASSERT_EQUAL(mw1[1], "style"s);

    const auto[mw2, status2] = server.MatchDocument("dog of magic imagination -wand"s, doc_id2);
    ASSERT_EQUAL(mw2.size(), 0);

    const auto[mw3, status3] = server.MatchDocument("fox with sandwich on the road light"s, doc_id3);
    ASSERT_EQUAL(mw3.size(), 2);
}

void TestSortingOfFoundDocumentsWithRelevance() {
    SearchServer server("и в на"s);
    //server.SetStopWords("и в на"s);

    server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::vector<Document>fds = server.FindTopDocuments("пушистый ухоженный кот"s);
    ASSERT_EQUAL(fds.size(), 3);
    ASSERT_EQUAL(fds[0].id, 1);
    ASSERT_EQUAL(fds[1].id, 0);
    ASSERT_EQUAL(fds[2].id, 2);
}

void TestDocumentsRatingCalculation() {
    SearchServer server("и в на"s);
    //server.SetStopWords("и в на"s);

    server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::vector<Document>fds = server.FindTopDocuments("пушистый ухоженный кот"s);
    ASSERT_EQUAL_HINT(fds[0].id, 1, "First doc must be #1"s);
    ASSERT_EQUAL_HINT(fds[0].rating, 5, "Rating calculation, doc.#1: (7+2+7)/3 = 5 for integers."s);

    ASSERT_EQUAL_HINT(fds[1].id, 0, "Second doc must be #0"s);
    ASSERT_EQUAL_HINT(fds[1].rating, 2, "Rating calculation, doc.#0: (8-3)/2 = 2 for integers."s);

    ASSERT_EQUAL_HINT(fds[2].id, 2, "Third doc must be #2"s);
    ASSERT_EQUAL_HINT(fds[2].rating, -1, "Rating calculation, doc.#2: (5-12+2+1)/4 = -1 for integers."s);
}

void TestRelevanceCalculationOfFoundDocs() {
    SearchServer server("и в на"s);
    //server.SetStopWords("и в на"s);

    server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::vector<Document>fds = server.FindTopDocuments("пушистый ухоженный кот"s);
    ASSERT_HINT(AreDoublesSame(fds[0].relevance, 0.866434), "Doc.#1, Sum of query words' IDF*TF: 1.386294*0.5 + 0.69314718*0 + 0.69314718*0.25"s);
    ASSERT_HINT(AreDoublesSame(fds[1].relevance, 0.173287), "Doc.#0, Sum of query words' IDF*TF: 1.386294*0 + 0.69314718*0 + 0.69314718*0.25"s);
    ASSERT_HINT(AreDoublesSame(fds[2].relevance, 0.173287), "Doc.#2, Sum of query words' IDF*TF: 1.386294*0 + 0.69314718*0.25 + 0.69314718*0"s);
}

void TestFilteringWithUserPredicate() {
    SearchServer server("и в на"s);
    //server.SetStopWords("и в на"s);

    server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::vector<Document>fds = server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus, int) { return document_id % 2 == 0; });
    ASSERT_EQUAL(fds.size(), 2);
    ASSERT_EQUAL(fds[0].id, 0);
    ASSERT_EQUAL(fds[1].id, 2);

    std::vector<Document>fds2 = server.FindTopDocuments("пушистый ухоженный кот"s, [](int, DocumentStatus, int rating) { return rating < 0; });
    ASSERT_EQUAL(fds2.size(), 1);
    ASSERT_EQUAL(fds2[0].id, 2);

}


void TestFilteringWithStatus() {
    SearchServer server("и в на"s);
    //server.SetStopWords("и в на"s);

    server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::vector<Document>fds1 = server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED);
    ASSERT_EQUAL(fds1.size(), 1);
    ASSERT_EQUAL(fds1[0].id, 3);

    std::vector<Document>fds2 = server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL);
    ASSERT_EQUAL(fds2.size(), 3);
    ASSERT_EQUAL(fds2[0].id, 1);
    ASSERT_EQUAL(fds2[1].id, 0);
    ASSERT_EQUAL(fds2[2].id, 2);


    SearchServer server1(""s);
    server1.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    server1.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server1.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server1.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::vector<Document>fds3 = server1.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED);
    ASSERT_EQUAL(fds3.size(), 1);
    ASSERT_EQUAL(fds3[0].id, 3);

    std::vector<Document>fds4 = server1.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL);
    ASSERT_EQUAL(fds4.size(), 3);
    ASSERT_EQUAL(fds4[0].id, 1);
    ASSERT_EQUAL(fds4[1].id, 2);
    ASSERT_EQUAL(fds4[2].id, 0);
}
