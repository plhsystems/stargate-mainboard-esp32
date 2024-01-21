#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "nvsjson.h"

class Settings
{
    public:
    enum class Entry
    {
        WSTAIsActive = 0,
        WSTASSID,
        WSTAPass,

        ClampLockedPWM,
        ClampReleasedPWM,
        RingSymbolBrightness,

        // Ring rotation calibration
        StepsPerRotation,
        RingHomeOffset,
        // Wormhole timeout
        GateOpenedTimeout,

        RampOnPercent,

        WormholeMaxBrightness,

        AnimPrelockDelayMS,
        AnimPostlockDelayMS,
        AnimPredialDelayMS,

        Mp3PlayerVolume,

        Count
    };

    private:
    Settings();

    public:
    // Singleton pattern
    Settings(Settings const&) = delete;
    void operator=(Settings const&) = delete;

    public:
    void Init();

    void Load();
    void Commit();

    int32_t GetValueInt32(Settings::Entry eEntry);
    NVSJSON_ESETRET SetValueInt32(Settings::Entry eEntry, bool bIsDryRun, int32_t s32NewValue);

    void GetValueString(Settings::Entry eEntry, char* out_value, size_t* length);
    NVSJSON_ESETRET SetValueString(Settings::Entry eEntry, bool bIsDryRun, const char* szValue);

    bool ImportJSON(const char* szJSON);
    char* ExportJSON();

    // Instance
    static Settings& getI()
    {
        static Settings instance;
        return instance;
    }
    private:
    static bool ValidateWifiPassword(const NVSJSON_SSettingEntry* pSettingEntry, const char* szValue);
    const NVSJSON_SSettingEntry m_sConfigEntries[(int)Settings::Entry::Count] =
    {
        // // WiFi Station related
        [(int)Settings::Entry::WSTAIsActive] =            NVSJSON_INITINT32_RNG("WSTA.IsActive",   "Wi-Fi is active",                        0,    0, 1, NVSJSON_EFLAGS_NeedsReboot),
        [(int)Settings::Entry::WSTASSID] =                NVSJSON_INITSTRING("WSTA.SSID",      "Wi-Fi (SSID)",                           "", NVSJSON_EFLAGS_NeedsReboot),
        [(int)Settings::Entry::WSTAPass] =                NVSJSON_INITSTRING_VALIDATOR("WSTA.Pass","Wi-Fi password",                         "", ValidateWifiPassword, (NVSJSON_EFLAGS)(NVSJSON_EFLAGS_Secret | NVSJSON_EFLAGS_NeedsReboot)),

        [(int)Settings::Entry::ClampLockedPWM] =          NVSJSON_INITINT32_RNG("Clamp.LockedPWM", "Servo motor locked PWM",             1250, 1000,   2000, NVSJSON_EFLAGS_None),
        [(int)Settings::Entry::ClampReleasedPWM] =        NVSJSON_INITINT32_RNG("Clamp.ReleasPWM", "Servo motor released PWM",           1000, 1000,   2000, NVSJSON_EFLAGS_None),
        [(int)Settings::Entry::RingSymbolBrightness] =    NVSJSON_INITINT32_RNG("Ring.SymBright",  "Symbol brightness",                    15,    3,     50, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::StepsPerRotation] =        NVSJSON_INITINT32_RNG("StepPerRot",      "How many step per rotation",         7334,    0,  64000, NVSJSON_EFLAGS_None),
        [(int)Settings::Entry::RingHomeOffset] =          NVSJSON_INITINT32_RNG("Ring.HomeOffset", "Offset relative to home sensor",      -55, -500,    500, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::GateOpenedTimeout] =       NVSJSON_INITINT32_RNG("GateTimeoutS",    "Timeout (s) before the gate close",   300,   10,  42*60, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::RampOnPercent] =           NVSJSON_INITINT32_RNG("Ramp.LightOn",    "Ramp illumination ON (percent)",       30,    0,    100, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::WormholeMaxBrightness] =   NVSJSON_INITINT32_RNG("WH.MaxBright",    "Maximum brightness for wormhole leds. (Warning: can cause voltage drop)", 200, 0, 255, NVSJSON_EFLAGS_None),

        // Animation delay
        [(int)Settings::Entry::AnimPrelockDelayMS] =      NVSJSON_INITINT32_RNG("dial.anim1",      "Delay before locking the chevron (ms)", 1250, 0, 6000, NVSJSON_EFLAGS_None),
        [(int)Settings::Entry::AnimPostlockDelayMS] =     NVSJSON_INITINT32_RNG("dial.anim2",      "Delay after locking the chevron (ms)",  1250, 0, 6000, NVSJSON_EFLAGS_None),
        [(int)Settings::Entry::AnimPredialDelayMS] =      NVSJSON_INITINT32_RNG("dial.anim3",      "Delay before starting to dial (ms)",    2500, 0, 10000, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::Mp3PlayerVolume] =         NVSJSON_INITINT32_RNG("sound.vol",       "Get Mp3 player volume",                 15,   0, 30, NVSJSON_EFLAGS_None),
    };
    NVSJSON_SHandle m_sSettingHandle;
    const NVSJSON_SConfig m_sSettingConfig = { .szPartitionName = "nvs", .pSettingEntries = m_sConfigEntries, .u32SettingEntryCount = (uint32_t)Settings::Entry::Count };
};

#endif
