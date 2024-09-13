#pragma once
//#include "search_server.h"

#include <vector>
#include <string>

//using namespace std;

template<typename Range>
struct Page {

    Range begin;
    Range end;
    size_t size;

    Page() : size(0) {}

    Page(Range begin_, Range end_, size_t size_) : begin(begin_), end(end_), size(size_) {}

};

template<typename Range>
struct Paginator {

    std::vector<Page<Range>> pages;

    Paginator(Range begin, Range end, size_t page_size) : pages() {

        size_t size = 0;
        auto page_begin = begin;

        for (auto i = begin; i < end; ++i) {
            if (size == page_size) {
                pages.push_back(Page(page_begin, i, size));
                size = 0;
                page_begin = i + 1;
            }
            ++size;
        }

        if (size != 0) {
            auto stop_on_doc = begin + pages.size() * page_size;
            pages.push_back(Page(stop_on_doc, stop_on_doc + size, size));
        }
    }

    auto begin() const {
        return pages.begin();
    }

    auto end() const {
        return pages.end();
    }

};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

