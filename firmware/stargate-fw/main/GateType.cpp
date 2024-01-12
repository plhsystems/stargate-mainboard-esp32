#include "GateType.hpp"

const GateType m_sGateTypes[] =
{
    [(int)GateType::Type::Kristian] = { GateType::Type::Kristian, 1, 39 },
    [(int)GateType::Type::Atlantis] = { GateType::Type::Atlantis, 1, 36 },
    [(int)GateType::Type::Universe] = { GateType::Type::Universe, 1, 36 }
};

GateType::GateType(Type type, uint8_t u8Minimum, uint8_t u8Maximum) :
    m_type(type),
    m_u8Minimum(u8Minimum),
    m_u8Maximum(u8Maximum)
{

}

const GateType& GateType::Get(Type type) const
{
    return m_sGateTypes[(int)type];
}