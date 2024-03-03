#pragma once

#include <cstdint>

namespace SGURingNS
{
    // Calculate the position
    double LEDIndexToDeg(int32_t s32LedIndex);
    int32_t ChevronIndexToLedIndex(int32_t s32ChevronNumber);
    int32_t SymbolToLedIndex(int32_t s32SymbolNumber);
    bool IsLEDIndexChevron(int32_t s32LedIndex);
}