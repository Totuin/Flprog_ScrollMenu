#pragma once
#include <Arduino.h>
#include <stdint.h>

// Не владеет массивами. Все функции вызываются только из pool().
class FlprogAbstractScrollMenuColumn {
public:
    FlprogAbstractScrollMenuColumn(uint16_t sourceSize, uint16_t outputSize)
        : _sourceSize(sourceSize), _outputSize(outputSize) {}
    virtual ~FlprogAbstractScrollMenuColumn() {}
    uint16_t getSourceSize() const { return _sourceSize; }
    uint16_t getOutputSize() const { return _outputSize; }
    virtual bool isReady() const = 0;
    virtual void copyRow(uint16_t sourceIndex, uint16_t outputIndex) = 0;
    virtual void clearRow(uint16_t outputIndex) = 0;
private:
    uint16_t _sourceSize, _outputSize;
};

template<class T> class FlprogScrollMenuTypedColumn : public FlprogAbstractScrollMenuColumn {
public:
    using ReadFunction = T (*)(uint16_t index);
    using WriteFunction = void (*)(T value, uint16_t index);
    FlprogScrollMenuTypedColumn(uint16_t sourceSize, uint16_t outputSize,
                               ReadFunction read = nullptr, WriteFunction write = nullptr)
        : FlprogAbstractScrollMenuColumn(sourceSize, outputSize), _read(read), _write(write) {}
    void setReadFunction(ReadFunction read) { _read = read; }
    void setWriteFunction(WriteFunction write) { _write = write; }
    void setEmptyValue(T value) { _empty = value; }
    bool isReady() const override { return _read && _write; }
    void copyRow(uint16_t sourceIndex, uint16_t outputIndex) override {
        if (isReady() && sourceIndex < getSourceSize() && outputIndex < getOutputSize())
            _write(_read(sourceIndex), outputIndex);
    }
    void clearRow(uint16_t outputIndex) override {
        if (_write && outputIndex < getOutputSize()) _write(_empty, outputIndex);
    }
private:
    ReadFunction _read;
    WriteFunction _write;
    T _empty = T();
};

using FlprogScrollMenuBoolColumn = FlprogScrollMenuTypedColumn<bool>;
using FlprogScrollMenuInt8Column = FlprogScrollMenuTypedColumn<int8_t>;
using FlprogScrollMenuUInt8Column = FlprogScrollMenuTypedColumn<uint8_t>;
using FlprogScrollMenuInt16Column = FlprogScrollMenuTypedColumn<int16_t>;
using FlprogScrollMenuUInt16Column = FlprogScrollMenuTypedColumn<uint16_t>;
using FlprogScrollMenuInt32Column = FlprogScrollMenuTypedColumn<int32_t>;
using FlprogScrollMenuUInt32Column = FlprogScrollMenuTypedColumn<uint32_t>;
using FlprogScrollMenuInt64Column = FlprogScrollMenuTypedColumn<int64_t>;
using FlprogScrollMenuUInt64Column = FlprogScrollMenuTypedColumn<uint64_t>;
using FlprogScrollMenuFloatColumn = FlprogScrollMenuTypedColumn<float>;
using FlprogScrollMenuDoubleColumn = FlprogScrollMenuTypedColumn<double>;
using FlprogScrollMenuStringColumn = FlprogScrollMenuTypedColumn<String>;

class FlprogScrollMenu {
public:
    explicit FlprogScrollMenu(uint16_t visibleRows) : _rows(visibleRows) {}
    // Реестр и объекты должны существовать всё время работы меню.
    // Колбэки можно установить после setColumns, но до pool.
    bool setColumns(FlprogAbstractScrollMenuColumn* const* columns, uint16_t count) {
        _columns = nullptr; _columnCount = 0; invalidate();
        if (!_rows || !columns || !count) return false;
        for (uint16_t i = 0; i < count; ++i) {
            if (!columns[i] || columns[i]->getOutputSize() < _rows) return false;
            for (uint16_t j = 0; j < i; ++j) if (columns[j] == columns[i]) return false;
        }
        _columns = columns; _columnCount = count; return true;
    }
    void setCount(int32_t count) { _requestedCount = count; }
    void setIndex(int32_t index) { _requestedIndex = index; }
    void refresh() { _dirty = true; }
    void setAutoRefresh(bool enable) { _autoRefresh = enable; }
    bool isReady() const { return _ready; }
    uint16_t getVisibleRows() const { return _rows; }
    uint16_t getCount() const { return _count; }
    uint16_t getFirstIndex() const { return _first; }
    uint16_t getVisibleCount() const { return _visible; }
    int32_t getIndex() const { return _index; }
    int32_t getSelectedRow() const { return _selectedRow; }
    bool isRowValid(uint16_t row) const { return _ready && row < _visible; }
    // true: выходные массивы переписаны. false: только выделение изменилось,
    // ничего не изменилось или конфигурация не готова (см. isReady()).
    bool pool() {
        if (!_columns || !_columnCount || !_rows) { invalidate(); return false; }
        uint16_t capacity = UINT16_MAX;
        for (uint16_t c = 0; c < _columnCount; ++c) {
            auto* col = _columns[c];
            if (!col || !col->isReady() || col->getOutputSize() < _rows) {
                invalidate(); return false;
            }
            if (col->getSourceSize() < capacity) capacity = col->getSourceSize();
        }
        const uint16_t count = _requestedCount <= 0 ? 0 :
            (_requestedCount > capacity ? capacity : static_cast<uint16_t>(_requestedCount));
        const int32_t index = count == 0 ? -1 : (_requestedIndex < 0 ? 0 :
            (_requestedIndex >= count ? static_cast<int32_t>(count) - 1 : _requestedIndex));
        uint16_t first = _ready ? _first : 0;
        const uint16_t maxFirst = count > _rows ? count - _rows : 0;
        if (first > maxFirst) first = maxFirst;
        if (index >= 0) {
            if (index < first) first = static_cast<uint16_t>(index);
            else if (index >= static_cast<int32_t>(first) + _rows)
                first = static_cast<uint16_t>(index - _rows + 1);
        }
        const uint16_t remaining = count - first;
        const uint16_t visible = remaining < _rows ? remaining : _rows;
        const bool copy = !_ready || _dirty || _autoRefresh || first != _first || visible != _visible;
        _count = count; _index = index; _first = first; _visible = visible;
        _selectedRow = index < 0 ? -1 : index - first;
        _ready = true; _dirty = false;
        if (!copy) return false;
        for (uint16_t row = 0; row < _rows; ++row) {
            for (uint16_t c = 0; c < _columnCount; ++c) {
                if (row < visible) _columns[c]->copyRow(static_cast<uint16_t>(first + row), row);
                else _columns[c]->clearRow(row);
            }
        }
        return true;
    }
private:
    void invalidate() {
        _ready = false; _dirty = true; _count = _first = _visible = 0;
        _index = _selectedRow = -1;
    }
    uint16_t _rows;
    FlprogAbstractScrollMenuColumn* const* _columns = nullptr;
    uint16_t _columnCount = 0;
    int32_t _requestedCount = 0, _requestedIndex = 0;
    uint16_t _count = 0, _first = 0, _visible = 0;
    int32_t _index = -1, _selectedRow = -1;
    bool _ready = false, _dirty = true, _autoRefresh = false;
};
