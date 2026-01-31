#include "PabloSGHW.hpp"
#include "led_strip.h"
#include "esp_log.h"
#include "driver/gpio.h"
// #include "driver/mcpwm.h"  // Legacy driver - commented out for ESP-IDF 6.1, not used yet
// #include "soc/mcpwm_periph.h"  // Legacy driver - commented out for ESP-IDF 6.1, not used yet
#include "driver/ledc.h"
#include "freertos/task.h"

PabloSGHW::PabloSGHW()
    : m_last_servo_position(0)
{
     // TODO: Implements
}

void PabloSGHW::Init()
{
    m_xMutexHandle = xSemaphoreCreateMutexStatic( &m_xMutexBuffer );
     // TODO: Implements
}

void PabloSGHW::SetChevronLight(EChevron chevron, bool state)
{
    // No such things on the pinky board.
}

void PabloSGHW::SetRampLight(double perc)
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

void PabloSGHW::SetWHPixel(uint32_t index, uint8_t red, uint8_t green, uint8_t blue)
{
     // TODO: Implements
}

void PabloSGHW::ClearAllWHPixels()
{
     // TODO: Implements
}

bool PabloSGHW::RefreshWHPixels()
{
     // TODO: Implements
     return false;
}

void PabloSGHW::SetSanityLED(bool state)
{
     // TODO: Implements
}

bool PabloSGHW::GetIsHomeSensorActive()
{
    return false; // TODO: Implements
}