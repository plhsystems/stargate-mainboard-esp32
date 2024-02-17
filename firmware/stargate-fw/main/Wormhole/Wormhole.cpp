#include <algorithm>
#include "Wormhole.hpp"
#include "misc-formula.h"
#include "misc-macro.h"
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
    m_bIsRunInitialized = false;
}

void Wormhole::OpeningAnimation()
{

}

void Wormhole::RunTicks()
{
    const float minF = 0.30f;
    const float maxF = 1.00f;
    const uint8_t u8MaxPWM = 220;

    if (!m_bIsRunInitialized) {
        // Random initialization
        for(int i = 0; i < m_pHAL->GetWHPixelCount(); i++) {
            SLedEffect* pLedEffect = &m_sLedEffects[i];
            pLedEffect->fOne = minF + (((esp_random() % 100) * 0.01f) * (maxF - minF));
            pLedEffect->bIsUp = (esp_random() % 2) != 0;
        }
        m_bIsRunInitialized = true;
    }

    for(int i = 0; i < m_pHAL->GetWHPixelCount(); i++) {
        SLedEffect* pLedEffect = &m_sLedEffects[i];

        const float fInc = /*0.0025f +*/ ( 0.0005f * (esp_random() % 100) );

        pLedEffect->fOne += fInc * (pLedEffect->bIsUp ? 1.0f : -1.0f);

        if (pLedEffect->fOne >= maxF) {
            pLedEffect->fOne = maxF;
            pLedEffect->bIsUp = false;
        }
        else if (pLedEffect->fOne <= minF) {
            pLedEffect->fOne = minF;
            pLedEffect->bIsUp = true;
        }

        // Linearize the output (from human perception POV)
        float fCorrValue = MISCFA_LinearizeLEDOutput(pLedEffect->fOne);

        // Make the outer ring glowing less
        switch(GetRing(i))
        {
            case 0:
                fCorrValue *= 0.2f;
                break;
            case 1:
                fCorrValue *= 0.6f;
                break;
            case 2:
                fCorrValue *= 0.9f;
                break;
            case 3:
                fCorrValue *= 1.0f;
                break;
        }

        m_pHAL->SetWHPixel(i, MISCMACRO_MAX((uint8_t)(fCorrValue*u8MaxPWM), 16), MISCMACRO_MAX((uint8_t)(fCorrValue*u8MaxPWM), 16), MISCMACRO_MIN(16+(uint8_t)(fCorrValue*(u8MaxPWM)), u8MaxPWM-16));
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


int32_t Wormhole::GetRing(int zeroBasedIndex)
{
    for(int j = 0; j < sizeof(m_pRing0_OneBased)/sizeof(m_pRing0_OneBased[0]); j++)
        if (m_pRing0_OneBased[j]-1 == zeroBasedIndex)
            return 0;
    for(int j = 0; j < sizeof(m_pRing1_OneBased)/sizeof(m_pRing1_OneBased[0]); j++)
        if (m_pRing1_OneBased[j]-1 == zeroBasedIndex)
            return 1;
    for(int j = 0; j < sizeof(m_pRing2_OneBased)/sizeof(m_pRing2_OneBased[0]); j++)
        if (m_pRing2_OneBased[j]-1 == zeroBasedIndex)
            return 2;
    for(int j = 0; j < sizeof(m_pRing3_OneBased)/sizeof(m_pRing3_OneBased[0]); j++)
        if (m_pRing3_OneBased[j]-1 == zeroBasedIndex)
            return 3;
    return 0;
}