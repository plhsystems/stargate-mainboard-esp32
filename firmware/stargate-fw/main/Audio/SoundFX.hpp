#ifndef _SOUNDFX_H_
#define _SOUNDFX_H_

class SoundFX
{
    public:
    enum class File
    {
        Nothing = 0,

        SGU_1_beginroll_mp3,
        SGU_2_chevlck_wav,
        SGU_3_chevlck2_wav,
        SGU_4_gateclos_mp3,
        SGU_5_gateopen_mp3,
        SGU_6_lggroll_mp3,
        SGU_7_lockfail_wav,
        SGU_10_wormhole_wav,

        Count
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
};

#endif
