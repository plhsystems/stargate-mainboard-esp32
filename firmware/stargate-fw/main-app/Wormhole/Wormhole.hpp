#pragma once

#include "../HW/SGHW_HAL.hpp"
#include "../SGResult.hpp"

class Wormhole
{
    public:
    static constexpr const char* TAG = "Wormhole";

    enum class EType
    {
        NormalSGU = 0,
        NormalSG1 = 1,
        //Blackhole = 2,
        //GlitchSGU = 3,

        Count
    };

    struct SLedEffect
    {
        float one;
        bool is_up;
    };
    private:
    // 0 = Exterior <====> 3 = interior
    enum class ERing
    {
        Ring0 = 0,
        Ring1,
        Ring2,
        Ring3,

        Count
    };

    enum class EDir
    {
        FadeIn,
        FadeOut
    };

    struct SRingEntry
    {
        const int8_t* ring;
        int32_t ring_count;
    };

    public:
    Wormhole(SGHW_HAL* hal, EType wormhole_type);

    void Begin();
    void OpeningAnimation();
    SGResult RunTicks();
    void ClosingAnimation();
    void End();

    private:
    EType m_wormhole_type;
    SGHW_HAL* m_hal;

    bool m_is_run_initialized = false;
    uint32_t m_max_brightness = 0;

    static constexpr uint32_t LEDEFFECT_COUNT = 48;
    SLedEffect m_led_effects[LEDEFFECT_COUNT];

    public:
    static bool Validatwormhole_type(EType wormhole_type);
    private:
    void Illuminatring(ERing ring, Wormhole::EDir dir);
    void ClearAll();

    // One based, but I should have made it 0 based like a respectable programmer.
    // do be fair that's how it's labelled on  the board.
    static constexpr int8_t m_ring0_one_based[] = { 1, 2, 3, 4, 5, 6, 7, 8, 13, 14, 15, 16, 17, 18, 19, 47, 48 };
    static constexpr int32_t RING0_COUNT = sizeof(m_ring0_one_based)/sizeof(m_ring0_one_based[0]);

    static constexpr int8_t m_ring1_one_based[] = { 9, 10, 11, 12, 20, 21, 22, 23, 24, 25, 26, 27, 31, 32, 33, 34, 35, 36 };
    static constexpr int32_t RING1_COUNT = sizeof(m_ring1_one_based)/sizeof(m_ring1_one_based[0]);

    static constexpr int8_t m_ring2_one_based[] = { 37, 38, 39, 40, 28, 29, 30, 42, 43 };
    static constexpr int32_t RING2_COUNT = sizeof(m_ring2_one_based)/sizeof(m_ring2_one_based[0]);

    static constexpr int8_t m_ring3_one_based[] = { 41, 44, 45, 46  };
    static constexpr int32_t RING3_COUNT = sizeof(m_ring3_one_based)/sizeof(m_ring3_one_based[0]);

    static constexpr SRingEntry m_ring_entries[(int)ERing::Count] =
    {
        [(int)ERing::Ring0] = { .ring = m_ring0_one_based, .ring_count = RING0_COUNT },
        [(int)ERing::Ring1] = { .ring = m_ring1_one_based, .ring_count = RING1_COUNT },
        [(int)ERing::Ring2] = { .ring = m_ring2_one_based, .ring_count = RING2_COUNT },
        [(int)ERing::Ring3] = { .ring = m_ring3_one_based, .ring_count = RING3_COUNT }
    };

    static ERing GetRing(int zero_based_index);

    public:
    inline static constexpr const char* m_texts[] =
    {
        [(int)EType::NormalSGU] = "Normal SGU",
        [(int)EType::NormalSG1] = "Normal SG1",
        //[(int)EType::Blackhole] = "Blackhole",
        //[(int)EType::GlitchSGU] = "Glitch SGU",
    };
    static_assert((int)EType::Count == sizeof(m_texts)/sizeof(m_texts[0]), "Text missmatch");

    inline static const char* GetTypeText(EType type) { return m_texts[(int)type]; }
};
