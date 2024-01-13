#include "BoardHW.hpp"
#include "HWConfig.hpp"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

void BoardHW::Init()
{

}

void BoardHW::SetChevronLight(Chevron eChevron, bool bState)
{
    // TODO: Fill
}

void BoardHW::SetRampLight(double dPerc)
{
    // TODO: Fill
}

bool BoardHW::GetIsHomeSensorActive()
{
    return false; // TODO: Fill
}

void BoardHW::MoveRelative(int32_t s32Target)
{
    // TODO: Fill
}

void BoardHW::PowerUpStepper()
{
    // TODO: Fill
}

void BoardHW::PowerDownStepper()
{
    // TODO: Fill
}

void BoardHW::PowerUpServo()
{
    // TODO: Fill
}

void BoardHW::SetServo(double dPosition)
{
    // TODO: Fill
}

void BoardHW::PowerDownServo()
{
    // TODO: Fill
}

int32_t BoardHW::WHGetPixelCount()
{
    return 0; // TODO: Fill
}

void BoardHW::WHSetPixel(uint32_t u32Index, uint8_t u8Red, uint8_t u8Green, uint8_t u8Blue)
{
    // TODO: Fill
}

void BoardHW::WHClearAllPixels()
{
    // TODO: Fill
}

void BoardHW::WHRefreshPixels()
{
    // TODO: Fill
}