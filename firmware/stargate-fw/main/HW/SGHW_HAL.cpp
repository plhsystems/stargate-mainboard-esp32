#include "SGHW_HAL.hpp"
#include "HWConfig.hpp"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

SGHW_HAL::SGHW_HAL()
{

}

void SGHW_HAL::Init()
{

}

void SGHW_HAL::SetChevronLight(Chevron eChevron, bool bState)
{

}

void SGHW_HAL::SetRampLight(double dPerc)
{

}

bool SGHW_HAL::GetIsHomeSensorActive()
{
    return false;
}

void SGHW_HAL::MoveChevronMotor(Chevron eChevron, MotorDirection motorDir)
{

}

void SGHW_HAL::MoveStepper(int32_t s32StepCount)
{

}

void SGHW_HAL::PowerUpStepper()
{

}

void SGHW_HAL::PowerDownStepper()
{

}

void SGHW_HAL::PowerUpServo()
{

}

void SGHW_HAL::SetServo(double dPosition)
{

}

void SGHW_HAL::PowerDownServo()
{

}

int32_t SGHW_HAL::GetWHPixelCount()
{
    return 0;
}

void SGHW_HAL::SetWHPixel(uint32_t u32Index, uint8_t u8Red, uint8_t u8Green, uint8_t u8Blue)
{

}

void SGHW_HAL::ClearAllWHPixels()
{

}

void SGHW_HAL::RefreshWHPixels()
{

}

void SGHW_HAL::SetSanityLED(bool bState)
{

}