#include "BoardHWBase.hpp"
#include "HWConfig.hpp"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

BoardHWBase::BoardHWBase()
{

}

void BoardHWBase::Init()
{

}

void BoardHWBase::SetChevronLight(Chevron eChevron, bool bState)
{

}

void BoardHWBase::SetRampLight(double dPerc)
{

}

bool BoardHWBase::GetIsHomeSensorActive()
{
    return false;
}

void BoardHWBase::MoveChevronMotor(Chevron eChevron, MotorDirection motorDir)
{

}

void BoardHWBase::MoveStepper(int32_t s32StepCount)
{

}

void BoardHWBase::PowerUpStepper()
{

}

void BoardHWBase::PowerDownStepper()
{

}

void BoardHWBase::PowerUpServo()
{

}

void BoardHWBase::SetServo(double dPosition)
{

}

void BoardHWBase::PowerDownServo()
{

}

int32_t BoardHWBase::GetWHPixelCount()
{
    return 0;
}

void BoardHWBase::SetWHPixel(uint32_t u32Index, uint8_t u8Red, uint8_t u8Green, uint8_t u8Blue)
{

}

void BoardHWBase::ClearAllWHPixels()
{

}

void BoardHWBase::RefreshWHPixels()
{

}