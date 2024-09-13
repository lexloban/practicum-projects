#pragma once

#include "json.h"


namespace json {
    class Builder;
    class KeyItemContext;
    class ArrayItemContext;
    class ValueAfterKeyItemContext;
    class ValueItem;


    class DictItemContext {
    public:
        virtual KeyItemContext& Key(std::string&& key) = 0;
        virtual Builder& EndDict() = 0;
    };

    class KeyItemContext {
    public:
        virtual ValueAfterKeyItemContext& Value(const Node& value) = 0;
        virtual DictItemContext& StartDict() = 0;
        virtual ArrayItemContext& StartArray() = 0;
    };

    class ValueAfterKeyItemContext : protected KeyItemContext {
    public:
        virtual KeyItemContext& Key(std::string&& key) = 0;
        virtual Builder& EndDict() = 0;
    };

    class ArrayItemContext {
    public:
        virtual ArrayItemContext& Value(const Node& value) = 0;
        virtual DictItemContext& StartDict() = 0;
        virtual ArrayItemContext& StartArray() = 0;
        virtual Builder& EndArray() = 0;

    };

    class ValueItem : protected ValueAfterKeyItemContext, protected ArrayItemContext {
    public:
        virtual ValueItem& Value(const Node& value) = 0;
        virtual Node Build() = 0;
        virtual Builder& EndArray() = 0;
    };


    class Builder
            : public DictItemContext, public ValueItem {
    public:
        Builder();

        KeyItemContext& Key(std::string&& key) override;
        ValueItem& Value(const Node& value) override;
        DictItemContext& StartDict() override;
        ArrayItemContext& StartArray() override;
        Builder& EndDict() override;
        Builder& EndArray() override;
        Node Build() override;

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
    };



}



