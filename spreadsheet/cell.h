#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell();

    void Set(std::string text, Position position);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
	
	
	
private:

	SheetInterface& sheet_;
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;
    std::unique_ptr<Impl> impl_;

	std::optional<Value> cash_;
	
	std::unique_set<Position> depends_on_cells_;
	std::unique_set<Position> cells_depend_;
	
	void ResetCash();
	
	void DependsOn(Position position);
	void NotDependsOn(Position position);

	void InsertDependency(Position position);
	void RemoveDependency(Position position);
	void InsertDependantCell(Position position);
	void RemoveDependantCell(Position position);
	
	void CheckCircularReference(std::unique_set<Position>& checked, Position throw_pos);
	
	
};