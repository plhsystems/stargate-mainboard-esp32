#ifndef _WORMHOLE_H_
#define _WORMHOLE_H_

class Wormhole
{
    public:
    enum class EType
    {
        NormalSGU = 0,
        NormalSG1 = 1,
        Hell = 2,
        Blackhole = 3,
        Strobe = 4,
        Glitch = 5,

        Count
    };

    public:
    Wormhole(EType eWormholeType);

    void OpenAnimation();
    void Run(volatile bool* pbIsCancelFlag);
    void CloseAnimation();

    void Shutdown();

    private:
    EType m_eWormholeType;
    public:
    static bool ValidateWormholeType(EType eWormholeType);
};

#endif