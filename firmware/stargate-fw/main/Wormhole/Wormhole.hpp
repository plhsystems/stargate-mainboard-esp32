#pragma once

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

    public:
    Wormhole(EType eWormholeType);

    void Begin();
    void OpeningAnimation();
    void RunTicks();
    void ClosingAnimation();
    void End();

    private:
    EType m_eWormholeType;
    public:
    static bool ValidateWormholeType(EType eWormholeType);

    inline static const char* m_szTexts[] =
    {
        [(int)EType::NormalSGU] = "Normal SGU",
        [(int)EType::NormalSG1] = "Normal SG1",
        [(int)EType::Blackhole] = "Blackhole",
        [(int)EType::GlitchSGU] = "Glitch SGU",
    };
    static_assert((int)EType::Count == sizeof(m_szTexts)/sizeof(m_szTexts[0]), "Text missmatch");

    inline static const char* GetTypeText(EType eType) { return m_szTexts[(int)eType]; }
};
