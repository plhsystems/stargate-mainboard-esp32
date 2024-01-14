#ifndef _GPIO_H_
#define _GPIO_H_

#include <cstdint>
#include "../Common/Chevron.hpp"
#include "HWConfig.hpp"
#include "led_strip.h"

class BoardHW final
{
    private:
    BoardHW() = delete;

    public:
    static void Init();

    static void SetChevronLight(Chevron eChevron, bool bState);
    static void SetRampLight(double dPerc);

    static bool GetIsHomeSensorActive();

    // Stepper.
    // Private functions
    static void PowerUpStepper();
    static void MoveRelative(int32_t s32Target);
    static void PowerDownStepper();

    static void PowerUpServo();
    static void SetServo(double dPosition);
    static void PowerDownServo();

    // Wormhole related
    static int32_t GetWHPixelCount();
    static void SetWHPixel(uint32_t u32Index, uint8_t u8Red, uint8_t u8Green, uint8_t u8Blue);
    static void ClearAllWHPixels();
    static void RefreshWHPixels();

    private:
    static led_strip_handle_t led_strip;
};

#endif