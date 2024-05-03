#include "PabloSGHW.hpp"
#include "led_strip.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"
#include "driver/ledc.h"
#include "freertos/task.h"

PabloSGHW::PabloSGHW()
    : m_dLastServoPosition(0)
{
     // TODO: Implements
}

void PabloSGHW::Init()
{
    m_xMutexHandle = xSemaphoreCreateMutexStatic( &m_xMutexBuffer );
     // TODO: Implements
}

void PabloSGHW::SetChevronLight(EChevron eChevron, bool bState)
{
    // No such things on the pinky board.
}

void PabloSGHW::SetRampLight(double dPerc)
{
     // TODO: Implements

}

void PabloSGHW::PowerUpStepper()
{
     // TODO: Implements

}

void PabloSGHW::StepStepperCW()
{
     // TODO: Implements
}

void PabloSGHW::StepStepperCCW()
{
     // TODO: Implements
}

void PabloSGHW::PowerDownStepper()
{
     // TODO: Implements
}

void PabloSGHW::PowerUpServo()
{
     // TODO: Implements
}

void PabloSGHW::SetServo(double dPosition)
{
     // TODO: Implements
}

void PabloSGHW::PowerDownServo()
{
     // TODO: Implements
}

// Wormhole related
int32_t PabloSGHW::GetWHPixelCount()
{
     // TODO: Implements
    return 0;
}

void PabloSGHW::SetWHPixel(uint32_t u32Index, uint8_t u8Red, uint8_t u8Green, uint8_t u8Blue)
{
     // TODO: Implements
}

void PabloSGHW::ClearAllWHPixels()
{
     // TODO: Implements
}

void PabloSGHW::RefreshWHPixels()
{
     // TODO: Implements
}

void PabloSGHW::SetSanityLED(bool bState)
{
     // TODO: Implements
}

bool PabloSGHW::GetIsHomeSensorActive()
{
    return false; // TODO: Implements
}