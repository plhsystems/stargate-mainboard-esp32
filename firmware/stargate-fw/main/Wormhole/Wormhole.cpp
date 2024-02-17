#include "Wormhole.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

Wormhole::Wormhole(EType eWormholeType) :
    m_eWormholeType(eWormholeType)
{

}

void Wormhole::Begin()
{

}

void Wormhole::OpeningAnimation()
{

}

void Wormhole::RunTicks()
{
    vTaskDelay(pdMS_TO_TICKS(10));
}

void Wormhole::ClosingAnimation()
{

}

void Wormhole::End()
{

}
