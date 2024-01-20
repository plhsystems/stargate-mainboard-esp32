#include "HWConfig.hpp"
#include "BoardHWBase.hpp"
#include "PinkyBoardHW.hpp"

class HW
{
    public:
    HW( const HW& ); // non construction-copyable
    HW& operator=( const HW& ); // non copyable

    static BoardHWBase* getI()
    {
        return &m_instance;
    }

    #if HWCONFIG_BOARD_CURRENT == HWCONFIG_BOARD_PINKYBOARD
    inline static PinkyBoardHW m_instance;
    #else
    inline static BoardHWBase m_instance;
    #endif
};