#include "GateFactory.hpp"

BaseGate& GateFactory::Get(BaseGate::Type type)
{
    switch(type)
    {
        case BaseGate::Type::Pegasus:
            return m_pegasusGate;
        case BaseGate::Type::Universe:
            return m_universeGate;
        default:
        case BaseGate::Type::MilkyWay:
            return m_milkyWayGate;
    }
}