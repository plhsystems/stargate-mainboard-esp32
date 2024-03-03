#pragma once

#include "HWConfig.hpp"
#include "SGHW_HAL.hpp"
#include "PinkySGHW.hpp"

class HW
{
    public:
    HW( const HW& ); // non construction-copyable
    HW& operator=( const HW& ); // non copyable

    static SGHW_HAL* getI()
    {
        return &m_instance;
    }

    #if HWCONFIG_BOARD_CURRENT == HWCONFIG_BOARD_PINKYBOARD
    inline static PinkySGHW m_instance;
    #else
    inline static SGHW_HAL m_instance;
    #endif
};