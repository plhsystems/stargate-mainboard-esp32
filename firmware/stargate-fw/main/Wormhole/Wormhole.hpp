#pragma once

#include "../HW/SGHW_HAL.hpp"

class Wormhole
{
    public:
    enum class EType
    {
        NormalSGU = 0,
        NormalSG1 = 1,
        Blackhole = 2,
        GlitchSGU = 3,

        Count
    };

    struct SLedEffect
    {
        float fOne;
        bool bIsUp;
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

    static constexpr uint32_t LEDEFFECT_COUNT = 48;
    SLedEffect m_sLedEffects[LEDEFFECT_COUNT];

    public:
    static bool ValidateWormholeType(EType eWormholeType);

    // 0 = Exterior <====> 3 = interior
    // One based, but I should have made it 0 based like a respectable programmer.
    // do be fair that's how it's labelled on  the board.
    static constexpr int m_pRing0_OneBased[] = { 1, 2, 3, 4, 5, 6, 7, 8, 13, 14, 15, 16, 17, 18, 19, 47, 48 };
    static constexpr int m_pRing1_OneBased[] = { 9, 10, 11, 12, 20, 21, 22, 23, 24, 25, 26, 27, 31, 32, 33, 34, 35, 36 };
    static constexpr int m_pRing2_OneBased[] = { 37, 38, 39, 40, 28, 29, 30, 42, 43 };
    static constexpr int m_pRing3_OneBased[] = { 41, 44, 45, 46  };

    inline static constexpr char* m_szTexts[] =
    {
        [(int)EType::NormalSGU] = "Normal SGU",
        [(int)EType::NormalSG1] = "Normal SG1",
        [(int)EType::Blackhole] = "Blackhole",
        [(int)EType::GlitchSGU] = "Glitch SGU",
    };
    static_assert((int)EType::Count == sizeof(m_szTexts)/sizeof(m_szTexts[0]), "Text missmatch");

    inline static const char* GetTypeText(EType eType) { return m_szTexts[(int)eType]; }
};
