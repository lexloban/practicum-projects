#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
// Реализуйте следующие методы:
        explicit Formula(std::string expression)
        try : ast_(ParseFormulaAST(std::move(expression))) {
        } catch (const FormulaException& exc) {
        }

        Value Evaluate(const SheetInterface& sheet) const override {
            try {
                return ast_.Execute(sheet);
            } catch (const FormulaError& exc) {
                return exc;
            }
        }

        std::string GetExpression() const override {
            std::stringstream result;
            ast_.PrintFormula(result);
            return result.str();
        }

        std::vector<Position> GetReferencedCells() const override {
            std::vector<Position> vec(ast_.GetCells().begin(), ast_.GetCells().end());

            std::sort(vec.begin(), vec.end());
            vec.erase(std::unique(vec.begin(), vec.end()), vec.end());

            return vec;
        }

    private:
        FormulaAST ast_;
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    } catch (FormulaException& exc) {
        throw exc;
    } catch (...) {
        throw FormulaException("Invalid formula");
    }

}
