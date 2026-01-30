#include <algorithm>
#include "Wormhole.hpp"
#include "../Settings.hpp"
#include "misc-formula.h"
#include "misc-macro.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"
#include "esp_log.h"

Wormhole::Wormhole(SGHW_HAL* hal, EType wormhole_type) :
    m_wormhole_type(wormhole_type),
    m_hal(hal)
{
    m_max_brightness = Settings::getI().GetValueInt32(Settings::Entry::WormholeMaxLight);

    for(int i = 0; i < LEDEFFECT_COUNT; i++) {
        SLedEffect* led_effect = &m_led_effects[i];

        led_effect->is_up = false;
        led_effect->one = 0.0f;
    }
}

void Wormhole::Begin()
{
    ClearAll();
    m_is_run_initialized = false;
}

void Wormhole::OpeningAnimation()
{
    ClearAll();
    Illuminatring(ERing::Ring0, EDir::FadeIn);
    ClearAll();
    Illuminatring(ERing::Ring1, EDir::FadeIn);
    ClearAll();
    Illuminatring(ERing::Ring2, EDir::FadeIn);
    ClearAll();
    Illuminatring(ERing::Ring3, EDir::FadeIn);
    vTaskDelay(pdMS_TO_TICKS(150));
    Illuminatring(ERing::Ring2, EDir::FadeIn);
    Illuminatring(ERing::Ring1, EDir::FadeIn);
    Illuminatring(ERing::Ring0, EDir::FadeIn);
}

bool Wormhole::RunTicks()
{
    const float minF = 0.30f;
    const float maxF = 1.00f;

    if (!m_is_run_initialized) {
        // Random initialization
        for(int i = 0; i < m_hal->GetWHPixelCount(); i++) {
            SLedEffect* led_effect = &m_led_effects[i];
            led_effect->one = minF + (((esp_random() % 100) * 0.01f) * (maxF - minF));
            led_effect->is_up = (esp_random() % 2) != 0;
        }
        m_is_run_initialized = true;
    }

    for(int i = 0; i < m_hal->GetWHPixelCount(); i++) 
    {
        SLedEffect* led_effect = &m_led_effects[i];

        const float inc = /*0.0025f +*/ ( 0.0005f * (esp_random() % 100) );

        led_effect->one += inc * (led_effect->is_up ? 1.0f : -1.0f);

        if (led_effect->one >= maxF) {
            led_effect->one = maxF;
            led_effect->is_up = false;
        }
        else if (led_effect->one <= minF) {
            led_effect->one = minF;
            led_effect->is_up = true;
        }

        // Linearize the output (from human perception POV)
        float corr_value = MISCFA_LinearizeLEDOutput(led_effect->one);

        // Make the outer ring glowing less
        constexpr float ring_corr_values[(int)Wormhole::ERing::Count] = { 0.1f, 0.6f, 0.9f, 1.0f };
        corr_value *= ring_corr_values[(int)GetRing(i)];

        const uint8_t pwm = (uint8_t)(corr_value*m_max_brightness);

        if (m_wormhole_type == EType::NormalSG1) {
            m_hal->SetWHPixel(i, MISCMACRO_MAX(pwm, 16), MISCMACRO_MAX(pwm, 16), MISCMACRO_MIN(16+pwm, m_max_brightness-16));
        }
        else if (m_wormhole_type == EType::NormalSGU) {
            m_hal->SetWHPixel(i, pwm, pwm, pwm);
        }
    }

    if (!m_hal->RefreshWHPixels())
    {
        ESP_LOGW(TAG, "Error during refresh, may be caused by power instability");
        return false;
    }

    // 25 HZ
    vTaskDelay(pdMS_TO_TICKS(40));

    return true;
}

void Wormhole::ClosingAnimation()
{
    Illuminatring(ERing::Ring3, EDir::FadeOut);
    Illuminatring(ERing::Ring2, EDir::FadeOut);
    Illuminatring(ERing::Ring1, EDir::FadeOut);
    Illuminatring(ERing::Ring0, EDir::FadeOut);

    // Clear all pixels
    ClearAll();
}

void Wormhole::End()
{
    ClearAll();
}

Wormhole::ERing Wormhole::GetRing(int zeroBasedIndex)
{
    for(int j = 0; j < sizeof(m_ring0_one_based)/sizeof(m_ring0_one_based[0]); j++)
        if (m_ring0_one_based[j]-1 == zeroBasedIndex)
            return Wormhole::ERing::Ring0;
    for(int j = 0; j < sizeof(m_ring1_one_based)/sizeof(m_ring1_one_based[0]); j++)
        if (m_ring1_one_based[j]-1 == zeroBasedIndex)
            return Wormhole::ERing::Ring1;
    for(int j = 0; j < sizeof(m_ring2_one_based)/sizeof(m_ring2_one_based[0]); j++)
        if (m_ring2_one_based[j]-1 == zeroBasedIndex)
            return Wormhole::ERing::Ring2;
    for(int j = 0; j < sizeof(m_ring3_one_based)/sizeof(m_ring3_one_based[0]); j++)
        if (m_ring3_one_based[j]-1 == zeroBasedIndex)
            return Wormhole::ERing::Ring3;
    return Wormhole::ERing::Ring0;
}

void Wormhole::ClearAll()
{
    m_hal->ClearAllWHPixels();
    m_hal->RefreshWHPixels();
}

void Wormhole::Illuminatring(Wormhole::ERing ring, Wormhole::EDir dir)
{
    for(int32_t s32 = 0; s32 <= 100; s32 += 10)
    {
        const float brig = ((dir == Wormhole::EDir::FadeOut) ? (100-s32) : s32) * 0.01f;
        const SRingEntry* ring_entries = &m_ring_entries[(int)ring];
        for(int i = 0; i < ring_entries->ring_count; i++) {
            m_hal->SetWHPixel(ring_entries->ring[i]-1, (uint8_t)(m_max_brightness * brig), (uint8_t)(m_max_brightness * brig), (uint8_t)(m_max_brightness * brig));
        }
        m_hal->RefreshWHPixels();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
