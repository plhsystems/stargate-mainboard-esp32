#pragma once

#include <cstdint>

class SoundFX
{
    public:
    enum class FileID
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
        SoundFile(const char* szName, const char* szDesc) :
            szName(szName),
            szDesc(szDesc)
        {

        }
        const char* szName;
        const char* szDesc;
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

    bool PlaySound(FileID soundFile, bool bRepeat);
    void StopSound();

    const SoundFile* GetFile(SoundFX::FileID file)
    {
        if ((int)file < 0 || (int)file >= (int)SoundFX::FileID::Count)
            return nullptr;
        return &m_soundFiles[(uint32_t)file];
    }

    uint32_t GetFileCount() { return SOUNDFILE_COUNT; }

    static SoundFX& getI()
    {
        static SoundFX instance;
        return instance;
    }
    private:
    static inline const SoundFile m_soundFiles[] =
    {
        [(int)FileID::SGU_1_beginroll]    = { "1_beginroll.wav",  "The stargate begins dialing" },
        [(int)FileID::SGU_2_chevlck]      = { "2_chevlck.wav",    "Chevron lock #1" },
        [(int)FileID::SGU_3_chevlck2]     = { "3_chevlck2.wav",   "Chevron lock #2" },
        [(int)FileID::SGU_4_gateclos]     = { "4_gateclos.wav",   "Stargate is closing" },
        [(int)FileID::SGU_5_gateopen]     = { "5_gateopen.wav",   "Stargate is opening" },
        [(int)FileID::SGU_6_lggroll]      = { "6_lggroll.wav",    "Stargate is spinning" },
        [(int)FileID::SGU_7_lockfail]     = { "7_lockfail.wav",   "Locking the last chevron doesn't work" },
        [(int)FileID::SGU_10_wormhole]    = { "10_wormhole.wav",  "Ambiant sound when the wormhole is open" }
    };
    static const int32_t SOUNDFILE_COUNT = sizeof(m_soundFiles)/sizeof(m_soundFiles[0]);
};

