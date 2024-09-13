#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    IncreaseSheetSize(pos);

    if (cells_.find(pos) == cells_.end()) {
        cells_[pos] = std::make_unique<Cell>(*this);
    }

    auto& cell = cells_.at(pos);

    auto buffer = cell->GetText();

    cell->Set(text);


    for (auto& ref_cell_pos : cell->GetReferencedCells()) {
        auto* ref_cell = dynamic_cast<Cell*>(GetCell(ref_cell_pos));
        if (ref_cell == nullptr) {
            cells_[ref_cell_pos] = std::make_unique<Cell>(*this);
            ref_cell = cells_.at(ref_cell_pos).get();
        }
        ref_cell->AddDependentCell(pos);
    }

    if (cell->IsReferenced()) {
        cell->Set(buffer);
        throw CircularDependencyException("CircularDependencyException");
    }

//    ++cells_count_;
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("InvalidPositionException");
    }
    if (pos.row <= sheet_size_.rows && pos.col <= sheet_size_.cols) {
        if (cells_.find(pos) != cells_.end()) {
            return cells_.at(pos).get();
        } else {
//            cells_[pos] = std::make_unique<Cell>(*this);
//            return cells_.at(pos).get();
            return nullptr;
        }
    }
    return nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("InvalidPositionException");
    }
    if (pos.row <= sheet_size_.rows && pos.col <= sheet_size_.cols) {
        if (cells_.find(pos) != cells_.end()) {
            return cells_.at(pos).get();
        } else {
//            cells_[pos] = std::make_unique<Cell>(*this);
//            return cells_.at(pos).get();
            return nullptr;
        }
    }
    return nullptr;

}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("InvalidPositionException");
    }
    if (pos.row <= sheet_size_.rows && pos.col <= sheet_size_.cols) {
        if (cells_.find(pos) != cells_.end()) {
            cells_.erase(pos);
            DecreaseSheetSize(pos);
        }
    }
}

Size Sheet::GetPrintableSize() const {
    if (cells_.empty()) {
        return {0, 0};
    }
    return {sheet_size_.rows + 1, sheet_size_.cols + 1};
}

void Sheet::PrintValues(std::ostream& output) const {
    if (cells_.empty()) {
        return;
    }
    for (int r = 0; r <= sheet_size_.rows; ++r) {
        for (int c = 0; c <= sheet_size_.cols; ++c) {
            Position pos{r, c};
            if (cells_.find(pos) != cells_.end()) {
                auto value = cells_.at(pos)->GetValue();

                if (std::holds_alternative<double>(value)) {
                    output << std::get<double>(value)
                           << ((c != sheet_size_.cols) ? "\t" : "");
                }
                if (std::holds_alternative<std::string>(value)) {
                    output << std::get<std::string>(value)
                           << ((c != sheet_size_.cols) ? "\t" : "");
                }
                if (std::holds_alternative<FormulaError>(value)) {
                    output << std::get<FormulaError>(value)
                           << ((c != sheet_size_.cols) ? "\t" : "");
                }
            } else if (c != sheet_size_.cols) {
                output << "\t";
            }
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    if (cells_.empty()) {
        return;
    }
    for (int r = 0; r <= sheet_size_.rows; ++r) {
        for (int c = 0; c <= sheet_size_.cols; ++c) {
            Position pos{r, c};
            if (cells_.find(pos) != cells_.end()) {
                output << cells_.at(pos)->GetText() << ((c != sheet_size_.cols) ? "\t" : "");
            } else if (c != sheet_size_.cols) {
                output << "\t";
            }
        }
        output << '\n';
    }
}

void Sheet::IncreaseSheetSize(Position new_set_pos) {
    if (!new_set_pos.IsValid()) {
        throw InvalidPositionException("InvalidPositionException");
    }
    sheet_size_.rows = std::max(sheet_size_.rows, new_set_pos.row);
    sheet_size_.cols = std::max(sheet_size_.cols, new_set_pos.col);
}
void Sheet::DecreaseSheetSize(Position last_clear_pos) {
    if (!last_clear_pos.IsValid()) {
        throw InvalidPositionException("InvalidPositionException");
    }

    if (cells_.size() == 0) {
        sheet_size_.rows = 0;
        sheet_size_.cols = 0;
        return;
    }

    auto last_row = last_clear_pos.row;
    if (last_row == sheet_size_.rows) {
        bool is_empty = true;
        while (is_empty) {
            for (int r = last_row; r >= 0; --r) {
                for (int c = 0; c <= sheet_size_.cols; ++c) {
                    Position pos{r, c};
                    if (cells_.find(pos) != cells_.end()) {
                        is_empty = false;
                    }
                }
                if (is_empty) {
                    --sheet_size_.rows;
                }
            }
        }
    }

    auto last_col = last_clear_pos.col;
    if (last_col == sheet_size_.cols) {
        bool is_empty = true;
        while (is_empty) {
            for (int r = 0; r <= sheet_size_.rows; ++r) {
                Position pos{r, last_col};
                if (cells_.find(pos) != cells_.end()) {
                    is_empty = false;
                }
            }
            if (is_empty) {
                --sheet_size_.cols;
            }
            --last_col;
        }
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}