#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    mutable std::unordered_map<Position, std::unique_ptr<Cell>, PositionHasher> cells_;

    Size sheet_size_;

    void IncreaseSheetSize(Position new_set_pos);
    void DecreaseSheetSize(Position last_clear_pos);
};