#pragma once

#include <stdint.h>

class GateType
{
    public:
    enum class Type
    {
        Kristian = 0,
        Atlantis = 1,
        Universe = 2,

        Count
    };

    GateType(Type type, uint8_t u8Minimum, uint8_t u8Maximum);

    const Type m_type;
    const uint8_t m_u8Minimum;
    const uint8_t m_u8Maximum;

    public:
    static GateType gt1;

    const GateType& Get(Type type) const;

    public:
    static const GateType m_sGateTypes[];
};


