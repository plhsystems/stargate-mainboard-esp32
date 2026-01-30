#pragma once

#include <iterator>

enum class SGResult
{
    OK = 0,

    Failed,

    Count
};

// Array of text descriptions for each enum value
const char* SGRESULT_TEXTS[] = {
    "OK",
    "Failed"
};

// Static assert to ensure the array size matches the enum count
static_assert( std::size(SGRESULT_TEXTS) == static_cast<size_t>(SGResult::Count), "SGResultText array size must match SGResult enum count");

// Helper function to get text for an enum value
const char* GetSGResultText(SGResult result)
{
    return SGRESULT_TEXTS[static_cast<size_t>(result)];
}

#define SGRESULT_OK_OR_RETURN(__ret__) do { \
    if ( SGResult::Ok != (__ret__) ) {
        return __ret__;
    }
} while(0)