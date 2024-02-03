#include "SGURing.hpp"

double SGURingNS::LEDIndexToDeg(int32_t s32LedIndex)
{
    if (s32LedIndex < 0 || s32LedIndex >= 45)
        return 0.0d;

    const double _1of54 = 360.0d / 54.0d;
    const double _1of9 = 360.0d / 9.0d;

    return (s32LedIndex / 5) * _1of9 + (((s32LedIndex % 5) != 0) ? 10 + ((s32LedIndex % 5)-1) * _1of54 : 0);
}

int32_t SGURingNS::ChevronIndexToLedIndex(int32_t s32ChevronNumber)
{
    if (s32ChevronNumber < 1 || s32ChevronNumber > 9)
        return 0;
    return (s32ChevronNumber - 1) * 5;
}

int32_t SGURingNS::SymbolToLedIndex(int32_t s32SymbolNumber)
{
    if (s32SymbolNumber < 1 || s32SymbolNumber > 36)
        return 0;
    const int32_t symbolIndex0Based = s32SymbolNumber - 1;
    return s32SymbolNumber + (symbolIndex0Based / 4);
}

bool SGURingNS::IsLEDIndexChevron(int32_t s32LedIndex)
{
    return ((s32LedIndex % 5) == 0);
}