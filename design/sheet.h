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

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;
    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;
    void PrintValues(std::ostream& output) const override;
    void PrintText(std::ostream& output) const override;

private:
    std::deque<std::deque<std::unique_ptr<Cell>>> cells_;
    size_t size_;

    void RefreshSize();
    
    size_t GetNullCount(size_t row);
    size_t GetEmptyRowsCount();
    
    void PrintValues(std::ostream& output, size_t row) const;
    void PrintText(std::ostream& output, size_t row) const;
    
    void ThrowInvalidPosition(Position pos) const;
};