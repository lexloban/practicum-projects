#pragma once

#include "common.h"
#include "formula.h"
//#include "sheet.h"

#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(const SheetInterface& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

    void AddDependentCell(Position pos);
    void InvalidateDependentCache();

private:
    class Impl {
    public:
        using Value = std::variant<std::string, double, FormulaError>;

        virtual void Set(std::string text) = 0;
        virtual std::string GetText() = 0;
        virtual Value GetValue(const SheetInterface& sheet) = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;

        virtual ~Impl() = default;
    };
    class EmptyImpl final : public Impl {
    public:
        EmptyImpl() = default;
        ~EmptyImpl() = default;

        std::string GetText() override;
        Value GetValue(const SheetInterface& sheet) override;
        std::vector<Position> GetReferencedCells() const override;
    private:
        void Set(std::string text) override;
    };
    class TextImpl final : public Impl {
    public:
        TextImpl() = default;
        ~TextImpl() = default;

        void Set(std::string text) override;
        std::string GetText() override;
        Value GetValue(const SheetInterface& sheet) override;
        std::vector<Position> GetReferencedCells() const override;

    private:
        std::string text_;
    };
    class FormulaImpl final : public Impl {
    public:
        FormulaImpl() = default;
        ~FormulaImpl() = default;

        void Set(std::string text) override;
        std::string GetText() override;
        Value GetValue(const SheetInterface& sheet) override;

        std::vector<Position> GetReferencedCells() const override;



    private:
        std::unique_ptr<FormulaInterface> formula_;
    };

    std::unique_ptr<Impl> impl_;
    const SheetInterface& sheet_;
    mutable std::optional<double> cache_;
    mutable bool is_done = false;
    
    std::unordered_set<Position, PositionHasher> dependent_cells_;
};

// Добавьте поля и методы для связи с таблицей, проверки циклических
// зависимостей, графа зависимостей и т. д.