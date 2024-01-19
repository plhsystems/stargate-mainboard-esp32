#include "GateFactory.hpp"

const BaseGate GateFactory::Get(BaseGate::Type type)
{
    switch(type)
    {
        case BaseGate::Type::Pegasus:
            return PegasusGate{};
        case BaseGate::Type::Universe:
            return UniverseGate{};
        default:
        case BaseGate::Type::MilkyWay:
            return MilkyWayGate{};
    }
}