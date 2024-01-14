#include "BaseGate.hpp"
#include "KristianGate.hpp"

class GateFactory
{
    public:
    static const BaseGate Get(BaseGate::Type type);
};