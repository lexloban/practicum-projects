#include "read_input_functions.h"

using namespace std;

std::string ReadLine() {
    std::string s;
    getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    ReadLine();
    return result;
}

ostream& operator<<(ostream& out, const Document& doc) {
    out << "{ document_id = " << doc.id << ", relevance = " << doc.relevance << ", rating = " << doc.rating << " }";
    return out;
}



