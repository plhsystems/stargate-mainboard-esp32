#include "BaseGate.hpp"
#include "MilkyWayGate.hpp"
#include "PegasusGate.hpp"
#include "UniverseGate.hpp"

class GateFactory
{
    private:
    GateFactory() = delete;

    public:
    static BaseGate& Get(GateGalaxy eGateGalaxy);

    private:
    static inline MilkyWayGate m_milkyWayGate;
    static inline PegasusGate m_pegasusGate;
    static inline UniverseGate m_universeGate;
};