#ifndef _SOUNDFX_H_
#define _SOUNDFX_H_

class SoundFX
{
    public:
    enum class File
    {
        SGU_1_beginroll = 0,
        SGU_2_chevlck,
        SGU_3_chevlck2,
        SGU_4_gateclos,
        SGU_5_gateopen,
        SGU_6_lggroll,
        SGU_7_lockfail,
        SGU_10_wormhole,

        Count
    };
    class SoundFile
    {
        public:
        SoundFile(const char* szName) :
            m_szName(szName)
        {

        }
        private:
        const char* m_szName;
    };

    private:
    SoundFX();

    public:
    // Singleton pattern
    SoundFX(SoundFX const&) = delete;
    void operator=(SoundFX const&) = delete;

    public:
    void Init();
    void Start();

    void PlaySound(File soundFile, bool bRepeat);
    void StopSound();

    static SoundFX& getI()
    {
        static SoundFX instance;
        return instance;
    }
    private:

    static inline const SoundFile m_soundFiles[] =
    {
        // TODO: Real data
        [(int)File::SGU_1_beginroll]    = { "1_beginroll.wav" },
        [(int)File::SGU_2_chevlck]      = { "2_chevlck.wav" },
        [(int)File::SGU_3_chevlck2]     = { "3_chevlck2.wav" },
        [(int)File::SGU_4_gateclos]     = { "4_gateclos.wav" },
        [(int)File::SGU_5_gateopen]     = { "5_gateopen.wav" },
        [(int)File::SGU_6_lggroll]      = { "6_lggroll.wav" },
        [(int)File::SGU_7_lockfail]     = { "7_lockfail.wav" },
        [(int)File::SGU_10_wormhole]    = { "10_wormhole.wav" }
    };
};

#endif
