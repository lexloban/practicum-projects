#pragma once


#include "document.h"
#include "paginator.h"

#include <iostream>

std::ostream& operator<<(std::ostream& out, const Document& doc);

std::string ReadLine();

int ReadLineWithNumber();

template <typename It>
std::ostream& operator<<(std::ostream& out, const Page<It> pages) {
    for (auto page = pages.begin; page < pages.end; page++) {
        out << *page;
    }
    return out;
}

