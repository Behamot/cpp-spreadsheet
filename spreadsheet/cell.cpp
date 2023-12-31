#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

#include <iostream>

class Cell::Impl {
    public:
      virtual Value GetValue(const SheetInterface& sheet) const = 0;
      virtual std::string GetText() const = 0;
      virtual std::vector<Position> GetReferencedCells() const = 0;
};

class Cell::EmptyImpl : public Impl {
    public:
    Value GetValue(const SheetInterface& ) const override {
        return "";
    }
    
    std::string GetText() const {
        return "";
    }

    std::vector<Position> GetReferencedCells() const {
        return {};
    }
};

class Cell::TextImpl : public Cell::Impl {
    public:
    TextImpl(std::string text): text_{std::move(text)} {
        
    }
    Value GetValue(const SheetInterface& ) const override {
        return text_[0] == ESCAPE_SIGN ? text_.substr(1) : text_;
    }
    
    std::string GetText() const {
        return text_;
    }

    std::vector<Position> GetReferencedCells() const {
        return {};
    }
    private:
    std::string text_;
};

class Cell::NumberImpl : public Cell::Impl {
    public:
    NumberImpl(std::string text, double value)
	: text_{std::move(text)}
	, value_{value} {
    }
    Value GetValue(const SheetInterface& ) const override {
        return value_;
    }
    
    std::string GetText() const {
        return text_;
    }

    std::vector<Position> GetReferencedCells() const {
        return {};
    }
    private:
    std::string text_;
	double value_;
};

class Cell::FormulaImpl : public Cell::Impl {
    public:
    FormulaImpl(std::string expression)
    : formula_{ ParseFormula(expression.substr(1))} {
        
    }
    
    Value GetValue(const SheetInterface& sheet) const override {
        auto ret = formula_->Evaluate(sheet);
        
        if(auto* pval = std::get_if<double>(&ret)) {
            return *pval;
        }
        
        if(auto* pval = std::get_if<FormulaError>(&ret)) {
            return std::move(*pval);
        }
        return "";
    }
    
    std::string GetText() const {
        return FORMULA_SIGN + formula_->GetExpression();
    }

    std::vector<Position> GetReferencedCells() const {
        return formula_->GetReferencedCells();
    }
    private:
    std::unique_ptr<FormulaInterface> formula_;
};

Cell::Cell(Sheet& sheet)
    :sheet_{ sheet } {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::~Cell() {}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::ContainsInDependencies(const std::unordered_set<Position>& positions) const {
	for(Position position: positions) {
		if(depends_on_cells_.count(position) == 0) {
			return false;
		}
	}
	return true;
}

void Cell::SetFormula(std::string text, Position position) {
	std::unique_ptr<FormulaImpl> impl = std::make_unique<FormulaImpl>(std::move(text));

	
	auto referenced_cells = impl->GetReferencedCells();
	std::unordered_set<Position> positions(
		referenced_cells.begin(), 
		remove_if(referenced_cells.begin(), referenced_cells.end(),
			[](Position position) {
				return !position.IsValid();
			}
		)
	);
	if(!ContainsInDependencies(positions)) {
		CheckCircularReference(positions, position);
	}
	impl_ = std::move(impl);
	RemoveDependency(position);
	InsertDependency(position, positions);
}

void Cell::RemoveDependency(Position this_position) {
	for(Position position: depends_on_cells_) {
		RemoveDependency(this_position, position);
	}
	depends_on_cells_.clear();
}

void Cell::RemoveDependency(Position this_position, Position other_position) {
	assert(depends_on_cells_.count(other_position) != 0);
	
	Cell* cell = sheet_.GetCell(other_position);
	
	assert(cell != nullptr);
	assert(cell->cells_depend_.count(this_position) != 0);
	
	cell->cells_depend_.erase(this_position);
}

void Cell::InsertDependency(Position this_position, Position other_position) {
	depends_on_cells_.insert(other_position);
	Cell& cell = sheet_.CreateCellIfNotCreated(other_position);
	cell.cells_depend_.insert(this_position);
}

void Cell::InsertDependency(Position this_position, const std::unordered_set<Position>& other_positions) {
	for(Position other_position: other_positions) {
		InsertDependency(this_position, other_position);
	}
}

void Cell::Set(std::string text, Position position) {	
	if(GetText() == text) {
		return;
	}
	
	ResetCash();
    if(text.size() > 1 && text[0] == '=') {
        SetFormula(std::move(text), position);
        return;
    }
    if(text.size()) {
		try {
			std::size_t pos;
			double result = std::stod(text, &pos);
			if (pos == text.size()) {
				impl_ = std::make_unique<NumberImpl>(std::move(text), result);
				RemoveDependency(position);
				return;
			}
		} catch(const std::invalid_argument&) {}
		
	}
	impl_ = std::make_unique<TextImpl>(std::move(text));
	RemoveDependency(position);
	return;
}

void Cell::Clear(Position position) {
	Set("", position);
}

Cell::Value Cell::GetValue() const {
	if(cash_) {
		return *cash_;
	}
    cash_ = impl_->GetValue(sheet_);
	return *cash_;
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

void Cell::ResetCash() {
	if(!cash_) {
		return;
	}
	cash_.reset();
	for(Position position: cells_depend_) {
		Cell* cell = sheet_.GetCell(position);
		if(cell) {
			cell->ResetCash();
		}
	}
}

void Cell::CheckCircularReference(std::unordered_set<Position>& positions, Position throw_position) {
	std::unordered_set<Position> already_check;
	CheckCircularReference(positions, already_check, throw_position);
}

void Cell::CheckCircularReference(std::unordered_set<Position>& positions, std::unordered_set<Position>& already_check, Position throw_position) {
	for(Position position: positions) {
		if(position == throw_position) {
			throw CircularDependencyException("Circular reference");
		}
		if(already_check.count(position)) {
			continue;
		}
		already_check.insert(position);
		Cell* cell = sheet_.GetCell(position);
		if(cell) {
			cell->CheckCircularReference(cell->depends_on_cells_, already_check, throw_position);
		}
	}
}
