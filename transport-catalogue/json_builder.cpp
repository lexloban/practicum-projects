#include "json_builder.h"

namespace json {
    Builder::Builder() {
        auto* root_ptr = &root_;
        nodes_stack_.push_back(std::move(root_ptr));
    }

    KeyItemContext& Builder::Key(std::string&& key) {
        if (!nodes_stack_.back()->IsDict()) {
        throw std::logic_error("Key outside the map!");
        }
        std::get<Dict>(nodes_stack_.back()->GetValue())[key];
        nodes_stack_.emplace_back(new Node(key));
        return *this;
    }

    ValueItem& Builder::Value(const Node& value)  {
        if ((nodes_stack_.size() == 1 && (root_.IsString() || root_.IsArray() || root_.IsDict()))
            || nodes_stack_.back()->IsDict()) {
            throw std::logic_error("Value outside key, array!");
        }
        ////////////////////////////////////////////////////////////////////////////////////////
        if (value.IsNull() && root_.IsNull() && nodes_stack_.size() == 1) {
            nodes_stack_.emplace_back(new Node{nullptr});
        }
        if (nodes_stack_.back()->IsArray()) {
            std::get<Array>(nodes_stack_.back()->GetValue()).reserve(nodes_stack_.back()->AsArray().size() + 1);
            std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(value);
        } else if (nodes_stack_.back()->IsString()) {
//            std::string key = nodes_stack_.back()->AsString();
            const std::string& key = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();
            std::get<Dict>(nodes_stack_.back()->GetValue()).at(key) = value;
        } else if (nodes_stack_.back()->IsNull() && nodes_stack_.size() == 1) {
            root_ = value;
        }

        return *this;
    }

    DictItemContext& Builder::StartDict() {
        if ((nodes_stack_.size() == 1 && (root_.IsString() || root_.IsArray() || root_.IsDict()))
            || nodes_stack_.back()->IsDict()) {
            throw std::logic_error("StartDict not after constructor!");
        }
        ////////////////////////////////////////////////////////////////////////////////////////

        if (nodes_stack_.back()->IsNull() && nodes_stack_.size() == 1) {
            root_ = Dict{};
            nodes_stack_.push_back(&root_);
            return *this;
        } else if (nodes_stack_.back()->IsString()) {
//            std::string key = nodes_stack_.back()->AsString();
            const std::string& key = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();
            std::get<Dict>(nodes_stack_.back()->GetValue()).at(key) = Dict{};
            nodes_stack_.emplace_back(&std::get<Dict>(nodes_stack_.back()->GetValue()).at(key));
        } else if (nodes_stack_.back()->IsArray()) {
            std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(Dict{});
            nodes_stack_.emplace_back(&std::get<Array>(nodes_stack_.back()->GetValue()).back());
        }

        return *this;
    }
    ArrayItemContext& Builder::StartArray() {
        if ((nodes_stack_.size() == 1 && (root_.IsString() || root_.IsArray() || root_.IsDict()))
            || nodes_stack_.back()->IsDict()) {
            throw std::logic_error("StartArray not after constructor!");
        }
        ////////////////////////////////////////////////////////////////////////////////////////

        if (nodes_stack_.back()->IsNull() && nodes_stack_.size() == 1) {
            root_ = Array{};
            nodes_stack_.push_back(&root_);
            return *this;
        } else if (nodes_stack_.back()->IsArray()) {
            std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(Array{});
            nodes_stack_.emplace_back(&std::get<Array>(nodes_stack_.back()->GetValue()).back());
        } else if (nodes_stack_.back()->IsString()) {
//            std::string key = nodes_stack_.back()->AsString();
            const std::string& key = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();
            std::get<Dict>(nodes_stack_.back()->GetValue()).at(key) = Array{};
            nodes_stack_.emplace_back(&std::get<Dict>(nodes_stack_.back()->GetValue()).at(key));
        }

        return *this;
    }

    Builder& Builder::EndDict() {
        if (!nodes_stack_.back()->IsDict()) {
            throw std::logic_error("StartDict not found!");
        }
        nodes_stack_.pop_back();
        return *this;
    }
    Builder& Builder::EndArray() {
        if (!nodes_stack_.back()->IsArray()) {
        throw std::logic_error("StartArray not found!");
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::Build() {
        if ((nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsDict()) && nodes_stack_.size() > 1) {
            throw std::logic_error("Build on incomplete object!");
        }
        if (nodes_stack_.back()->IsNull() && nodes_stack_.size() == 1) {
            throw std::logic_error("Build on incomplete object!");
        }
        return root_;
    }


}

