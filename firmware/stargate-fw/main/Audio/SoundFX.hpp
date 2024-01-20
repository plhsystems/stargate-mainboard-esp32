#ifndef _SOUNDFX_H_
#define _SOUNDFX_H_

class SoundFX
{
    public:
    enum class File
    {
        SGU_1_beginroll_mp3 = 0,
        SGU_2_chevlck_wav,
        SGU_3_chevlck2_wav,
        SGU_4_gateclos_mp3,
        SGU_5_gateopen_mp3,
        SGU_6_lggroll_mp3,
        SGU_7_lockfail_wav,
        SGU_10_wormhole_wav,

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
        [(int)File::SGU_1_beginroll_mp3] = { "" },
        [(int)File::SGU_2_chevlck_wav] = { "" },
        [(int)File::SGU_3_chevlck2_wav] = { "" },
        [(int)File::SGU_4_gateclos_mp3] = { "" },
        [(int)File::SGU_5_gateopen_mp3] = { "" },
        [(int)File::SGU_6_lggroll_mp3] = { "" },
        [(int)File::SGU_7_lockfail_wav] = { "" },
        [(int)File::SGU_10_wormhole_wav] = { "" }
    };
};

#endif
