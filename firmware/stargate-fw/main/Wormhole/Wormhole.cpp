#include <algorithm>
#include "Wormhole.hpp"
#include "../Settings.hpp"
#include "misc-formula.h"
#include "misc-macro.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"

Wormhole::Wormhole(SGHW_HAL* pHAL, EType eWormholeType) :
    m_eWormholeType(eWormholeType),
    m_pHAL(pHAL)
{
    m_u32MaxBrightness = Settings::getI().GetValueInt32(Settings::Entry::WormholeMaxLight);

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
    m_pHAL->ClearAllWHPixels();
    m_pHAL->RefreshWHPixels();

    IlluminateRing(ERing::Ring0);
    IlluminateRing(ERing::Ring1);
    IlluminateRing(ERing::Ring2);
    IlluminateRing(ERing::Ring3);
    vTaskDelay(pdMS_TO_TICKS(400));
    IlluminateRing(ERing::Ring2);
    IlluminateRing(ERing::Ring1);
    IlluminateRing(ERing::Ring0);
}

void Wormhole::RunTicks()
{
    const float minF = 0.30f;
    const float maxF = 1.00f;

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
        constexpr float fltRingCorrValues[(int)Wormhole::ERing::Count] = { 0.1f, 0.6f, 0.9f, 1.0f };
        fCorrValue *= fltRingCorrValues[(int)GetRing(i)];

        m_pHAL->SetWHPixel(i, MISCMACRO_MAX((uint8_t)(fCorrValue*m_u32MaxBrightness), 16), MISCMACRO_MAX((uint8_t)(fCorrValue*m_u32MaxBrightness), 16), MISCMACRO_MIN(16+(uint8_t)(fCorrValue*(m_u32MaxBrightness)), m_u32MaxBrightness-16));
    }
    m_pHAL->RefreshWHPixels();

    // 50 HZ
    vTaskDelay(pdMS_TO_TICKS(20));
}

void Wormhole::ClosingAnimation()
{
    IlluminateRing(ERing::Ring3);
    IlluminateRing(ERing::Ring2);
    IlluminateRing(ERing::Ring1);

    // Clear all pixels
    m_pHAL->ClearAllWHPixels();
    m_pHAL->RefreshWHPixels();
}

void Wormhole::End()
{
    m_pHAL->ClearAllWHPixels();
    m_pHAL->RefreshWHPixels();
}

Wormhole::ERing Wormhole::GetRing(int zeroBasedIndex)
{
    for(int j = 0; j < sizeof(m_pRing0_OneBased)/sizeof(m_pRing0_OneBased[0]); j++)
        if (m_pRing0_OneBased[j]-1 == zeroBasedIndex)
            return Wormhole::ERing::Ring0;
    for(int j = 0; j < sizeof(m_pRing1_OneBased)/sizeof(m_pRing1_OneBased[0]); j++)
        if (m_pRing1_OneBased[j]-1 == zeroBasedIndex)
            return Wormhole::ERing::Ring1;
    for(int j = 0; j < sizeof(m_pRing2_OneBased)/sizeof(m_pRing2_OneBased[0]); j++)
        if (m_pRing2_OneBased[j]-1 == zeroBasedIndex)
            return Wormhole::ERing::Ring2;
    for(int j = 0; j < sizeof(m_pRing3_OneBased)/sizeof(m_pRing3_OneBased[0]); j++)
        if (m_pRing3_OneBased[j]-1 == zeroBasedIndex)
            return Wormhole::ERing::Ring3;
    return Wormhole::ERing::Ring0;
}

void Wormhole::IlluminateRing(Wormhole::ERing eRing)
{
    const uint32_t whiteMax = m_u32MaxBrightness;

    for(int32_t s32 = 0; s32 <= 100; s32 += 5)
    {
        const float brig = s32 * 0.01f;

        const SRingEntry* psRingEntries = &m_sRingEntries[(int)eRing];

        for(int i = 0; i < psRingEntries->u32RingCount; i++)
        {
            m_pHAL->SetWHPixel(psRingEntries->pRing[i]-1, (uint8_t)(whiteMax - m_u32MaxBrightness * brig), (uint8_t)(whiteMax - m_u32MaxBrightness * brig), (uint8_t)(whiteMax - m_u32MaxBrightness * brig));
        }

        m_pHAL->RefreshWHPixels();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
