#include "Wormhole.hpp"
#include "misc-formula.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"

Wormhole::Wormhole(SGHW_HAL* pHAL, EType eWormholeType) :
    m_eWormholeType(eWormholeType),
    m_pHAL(pHAL)
{
    for(int i = 0; i < LEDEFFECT_COUNT; i++) {
        SLedEffect* pLedEffect = &m_sLedEffects[i];

        pLedEffect->bIsUp = false;
        pLedEffect->fOne = 0.0f;
    }
}

void Wormhole::Begin()
{
    m_pHAL->ClearAllWHPixels();
    m_pHAL->RefreshWHPixels();
}

void Wormhole::OpeningAnimation()
{

}

void Wormhole::RunTicks()
{
    const float minF = 0.10f;
    const float maxF = 1.00f;
    const uint8_t u8MaxPWM = 180;

    for(int i = 0; i < m_pHAL->GetWHPixelCount(); i++) {
        SLedEffect* pLedEffect = &m_sLedEffects[i];

        const float fInc = 0.005f + ( 0.001f * (esp_random() % 100) );

        pLedEffect->fOne += fInc * (pLedEffect->bIsUp ? 1.0f : -1.0f);

        if (pLedEffect->fOne >= maxF) {
            pLedEffect->fOne = maxF;
            pLedEffect->bIsUp = false;
        }
        else if (pLedEffect->fOne <= minF) {
            pLedEffect->fOne = minF;
            pLedEffect->bIsUp = true;
        }

        const float fCorrValue = MISCFA_LinearizeLEDOutput(pLedEffect->fOne);
        
        m_pHAL->SetWHPixel(i, fCorrValue*u8MaxPWM, fCorrValue*u8MaxPWM, fCorrValue*u8MaxPWM);
    }

    m_pHAL->RefreshWHPixels();
    vTaskDelay(pdMS_TO_TICKS(20));
}

void Wormhole::ClosingAnimation()
{

}

void Wormhole::End()
{
    m_pHAL->ClearAllWHPixels();
    m_pHAL->RefreshWHPixels();
}
