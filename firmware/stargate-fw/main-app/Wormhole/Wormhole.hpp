#pragma once

#include "../HW/SGHW_HAL.hpp"

class Wormhole
{
    public:
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
        float fOne;
        bool bIsUp;
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
        const int8_t* pRing;
        int32_t u32RingCount;
    };

    public:
    Wormhole(SGHW_HAL* pHAL, EType eWormholeType);

    void Begin();
    void OpeningAnimation();
    void RunTicks();
    void ClosingAnimation();
    void End();

    private:
    EType m_eWormholeType;
    SGHW_HAL* m_pHAL;

    bool m_bIsRunInitialized = false;
    uint32_t m_u32MaxBrightness = 0;

    static constexpr uint32_t LEDEFFECT_COUNT = 48;
    SLedEffect m_sLedEffects[LEDEFFECT_COUNT];

    public:
    static bool ValidateWormholeType(EType eWormholeType);
    private:
    void IlluminateRing(ERing eRing, Wormhole::EDir dir);
    void ClearAll();

    // One based, but I should have made it 0 based like a respectable programmer.
    // do be fair that's how it's labelled on  the board.
    static constexpr int8_t m_pRing0_OneBased[] = { 1, 2, 3, 4, 5, 6, 7, 8, 13, 14, 15, 16, 17, 18, 19, 47, 48 };
    static constexpr int32_t RING0_COUNT = sizeof(m_pRing0_OneBased)/sizeof(m_pRing0_OneBased[0]);

    static constexpr int8_t m_pRing1_OneBased[] = { 9, 10, 11, 12, 20, 21, 22, 23, 24, 25, 26, 27, 31, 32, 33, 34, 35, 36 };
    static constexpr int32_t RING1_COUNT = sizeof(m_pRing1_OneBased)/sizeof(m_pRing1_OneBased[0]);

    static constexpr int8_t m_pRing2_OneBased[] = { 37, 38, 39, 40, 28, 29, 30, 42, 43 };
    static constexpr int32_t RING2_COUNT = sizeof(m_pRing2_OneBased)/sizeof(m_pRing2_OneBased[0]);

    static constexpr int8_t m_pRing3_OneBased[] = { 41, 44, 45, 46  };
    static constexpr int32_t RING3_COUNT = sizeof(m_pRing3_OneBased)/sizeof(m_pRing3_OneBased[0]);

    static constexpr SRingEntry m_sRingEntries[(int)ERing::Count] =
    {
        [(int)ERing::Ring0] = { .pRing = m_pRing0_OneBased, .u32RingCount = RING0_COUNT },
        [(int)ERing::Ring1] = { .pRing = m_pRing1_OneBased, .u32RingCount = RING1_COUNT },
        [(int)ERing::Ring2] = { .pRing = m_pRing2_OneBased, .u32RingCount = RING2_COUNT },
        [(int)ERing::Ring3] = { .pRing = m_pRing3_OneBased, .u32RingCount = RING3_COUNT }
    };

    static ERing GetRing(int zeroBasedIndex);

    public:
    inline static constexpr const char* m_szTexts[] =
    {
        [(int)EType::NormalSGU] = "Normal SGU",
        [(int)EType::NormalSG1] = "Normal SG1",
        //[(int)EType::Blackhole] = "Blackhole",
        //[(int)EType::GlitchSGU] = "Glitch SGU",
    };
    static_assert((int)EType::Count == sizeof(m_szTexts)/sizeof(m_szTexts[0]), "Text missmatch");

    inline static const char* GetTypeText(EType eType) { return m_szTexts[(int)eType]; }
};
