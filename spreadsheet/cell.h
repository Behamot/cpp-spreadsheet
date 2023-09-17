#pragma once

#include "common.h"
#include "formula.h"


#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text, Position position);
    void Clear(Position position);

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
	Sheet& sheet_;
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
	class NumberImpl;
	std::unique_ptr<Impl> impl_;

	mutable std::optional<Value> cash_;

	std::unordered_set<Position> depends_on_cells_{};
	std::unordered_set<Position> cells_depend_{};

	void ResetCash();

	void InsertDependency(Position this_position, Position other_position);
	void InsertDependency(Position this_position, const std::unordered_set<Position>& other_positions);
	void RemoveDependency(Position this_position, Position other_position);
	void RemoveDependency(Position this_position);

	void CheckCircularReference(std::unordered_set<Position>& positions, std::unordered_set<Position>& already_check, Position throw_position);
	void CheckCircularReference(std::unordered_set<Position>& positions, Position throw_position);
	
	void SetFormula(std::string text, Position position);
	bool ContainsInDependencies(const std::unordered_set<Position>& positions) const;
};