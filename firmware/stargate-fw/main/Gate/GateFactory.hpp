#include "BaseGate.hpp"
#include "MilkyWayGate.hpp"
#include "PegasusGate.hpp"
#include "UniverseGate.hpp"

class GateFactory
{
    public:
    static const BaseGate Get(BaseGate::Type type);
};