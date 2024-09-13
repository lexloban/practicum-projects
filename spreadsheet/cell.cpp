#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>

class Formula;

// ---------------- EmptyImpl ----------------
void Cell::EmptyImpl::Set(std::string text) {}
std::string Cell::EmptyImpl::GetText() {
    return "";
}
Cell::Impl::Value Cell::EmptyImpl::GetValue(const SheetInterface& sheet) {
    return "";
}
std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
    return {};
}

// ---------------- TextImpl ----------------

void Cell::TextImpl::Set(std::string text) {
    text_ = std::move(text);
}
std::string Cell::TextImpl::GetText() {
    return text_;
}
Cell::Impl::Value Cell::TextImpl::GetValue(const SheetInterface& sheet) {
    if (text_[0] == '\'') {
        return std::string(text_.begin() + 1, text_.end());
    }
    return text_;
}
std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
    return {};
}

// ---------------- FormulaImpl ----------------

void Cell::FormulaImpl::Set(std::string text) {
    formula_ = ParseFormula(text);
}
std::string Cell::FormulaImpl::GetText() {
    if (formula_ == nullptr) {
        return "=";
    }
    return '=' + formula_->GetExpression();
}
Cell::Impl::Value Cell::FormulaImpl::GetValue(const SheetInterface& sheet) {
    if (formula_ == nullptr) {
        return "=";
    }
    if (std::holds_alternative<FormulaError>(formula_->Evaluate(sheet))) {
        return std::get<FormulaError>(formula_->Evaluate(sheet));
    }
    return std::get<double>(formula_->Evaluate(sheet));
}
std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

// ---------------- Cell ----------------

Cell::Cell(const SheetInterface& sheet) : sheet_(sheet) {
    impl_ = std::make_unique<EmptyImpl>();
}
Cell::~Cell() = default;

void Cell::Set(std::string text) {
    if (cache_.has_value()) {
        InvalidateDependentCache();
    }

    if (text[0] == '=') {
        impl_ = std::make_unique<FormulaImpl>();
        if (text.size() > 1) {
            impl_->Set(std::string(text.begin() + 1, text.end()));
        }
    } else {
        impl_ = std::make_unique<TextImpl>();
        impl_->Set(text);
    }

}
void Cell::Clear() {
    impl_ = nullptr;
}

Cell::Value Cell::GetValue() const {
    if (cache_.has_value()) {
        return cache_.value();
    }

    auto result = impl_->GetValue(sheet_);

    if (std::holds_alternative<double>(result)) {
        cache_ = std::get<double>(result);
    }

    return result;
}
std::string Cell::GetText() const {
    return impl_->GetText();
}
std::vector<Position> Cell::GetReferencedCells() const {
    if (impl_ == nullptr) {
        return {};
    }
    return impl_->GetReferencedCells();
}
bool Cell::IsReferenced() const {
    if (is_done) {
        return true;
    }
    is_done = true;

    for (auto& pos : GetReferencedCells()) {
        const Cell* cell = dynamic_cast<const Cell*>(sheet_.GetCell(pos));

        if (cell != nullptr) {
            if (cell->IsReferenced()) {
                is_done = false;
                return true;
            }
        }
    }
    is_done = false;

    return false;
}

void Cell::AddDependentCell(Position pos) {
    dependent_cells_.insert(pos);
}

void Cell::InvalidateDependentCache() {
    cache_.reset();
    for (auto& pos : dependent_cells_) {
        Cell* cell = dynamic_cast<Cell*>(const_cast<SheetInterface&>(sheet_).GetCell(pos));
        cell->InvalidateDependentCache();
    }
}