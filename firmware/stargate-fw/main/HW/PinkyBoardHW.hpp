#pragma once

#include <cstdint>
#include "BoardHWBase.hpp"
#include "led_strip.h"

class PinkyBoardHW : public BoardHWBase
{

    private:
    led_strip_handle_t led_strip;
};