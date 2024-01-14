#include "GateFactory.hpp"
#include "BaseGate.hpp"
#include "KristianGate.hpp"
#include "AtlantisGate.hpp"
#include "UniverseGate.hpp"

const BaseGate GateFactory::Get(BaseGate::Type type)
{
    switch(type)
    {
        case BaseGate::Type::Atlantis:
            return AtlantisGate{};
        case BaseGate::Type::Universe:
            return UniverseGate{};
        default:
        case BaseGate::Type::Kristian:
            return KristianGate{};
    }
}