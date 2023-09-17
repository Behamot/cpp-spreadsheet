#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <deque>
#include <memory>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const Cell* GetCell(Position pos) const override;
    Cell* GetCell(Position pos) override;
    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
	
	Cell& CreateCellIfNotCreated(Position pos);

private:
    std::deque<std::deque<std::unique_ptr<Cell>>> cells_; //Ќе было времени поправить, но можно использовать unordered_map

    Size size_;
    void RefreshSize();

    size_t GetNullCount(size_t row);
    size_t GetEmptyRowsCount(); 
	
	template <typename GetFunc>
	void PrintRow(std::ostream& output, size_t row, GetFunc getter) const;
	
	template <typename GetFunc>
	void PrintCells(std::ostream& output, GetFunc getter) const;
    
    void ThrowOnInvalidPosition(Position pos) const;
};