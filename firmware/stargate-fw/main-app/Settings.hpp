#pragma once

#include "nvsjson.h"
#include "Gate/BaseGate.hpp"

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
        RingSymbolLight,

        // Ring rotation calibration
        StepsPerRotation,

        RingCalibTimeout,
        RingHomeOffset,
        RingHomeGapRange,
        // Wormhole timeout
        GateOpenedTimeout,

        RampOnPercent,

        WormholeMaxLight,

        AnimPrelockDelayMS,
        AnimPostlockDelayMS,
        AnimPredialDelayMS,

        SoundFXVolume,

        GateGalaxy,

        Count
    };

    public:
    Settings() = default;

    public:
    // Singleton pattern
    Settings(Settings const&) = delete;
    void operator=(Settings const&) = delete;

    public:
    void Init();

    void Load();
    void Commit();

    int32_t GetValueInt32(Settings::Entry entry);
    NVSJSON_ESETRET SetValueInt32(Settings::Entry entry, int32_t new_value);
    NVSJSON_ESETRET SetValueInt32(Settings::Entry entry, bool is_dry_run, int32_t new_value);

    void GetValueString(Settings::Entry entry, char* out_value, size_t* length);
    NVSJSON_ESETRET SetValueString(Settings::Entry entry, bool is_dry_run, const char* value);

    double GetValueDouble(Settings::Entry entry);
    NVSJSON_ESETRET SetValueDouble(Settings::Entry entry, bool is_dry_run, double value);
    NVSJSON_ESETRET SetValueDouble(Settings::Entry entry, double value);

    bool ImportJSON(const char* json);
    char* ExportJSON();

    // Instance
    static Settings& getI()
    {
        static Settings instance;
        return instance;
    }
    private:
    static bool ValidateWifiPassword(const NVSJSON_SSettingEntry* setting_entry, const char* value);
    const NVSJSON_SSettingEntry m_config_entries[(int)Settings::Entry::Count] =
    {
        // // WiFi Station related
    //                                                                                                                                      DEFAULT MIN MAX
        [(int)Settings::Entry::WSTAIsActive] =            NVSJSON_INITINT32_RNG("WSTA.IsActive",   "Wi-Fi is active",                       0,    0, 1, NVSJSON_EFLAGS_NeedsReboot),
        [(int)Settings::Entry::WSTASSID] =                NVSJSON_INITSTRING("WSTA.SSID",          "Wi-Fi (SSID)",                          "", NVSJSON_EFLAGS_NeedsReboot),
        [(int)Settings::Entry::WSTAPass] =                NVSJSON_INITSTRING_VAL("WSTA.Pass",      "Wi-Fi password",                        "", ValidateWifiPassword, (NVSJSON_EFLAGS)(NVSJSON_EFLAGS_Secret | NVSJSON_EFLAGS_NeedsReboot)),

        [(int)Settings::Entry::ClampLockedPWM] =          NVSJSON_INITDOUBLE_RNG("Clamp.LockedPWM", "Servo motor locked PWM",               0.78f, 0.0f,   1.0f, NVSJSON_EFLAGS_None),
        [(int)Settings::Entry::ClampReleasedPWM] =        NVSJSON_INITDOUBLE_RNG("Clamp.ReleasPWM", "Servo motor released PWM",             0.45f, 0.0f,   1.0f, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::RingSymbolLight] =         NVSJSON_INITINT32_RNG("SGUR.SymLight",   "Symbol brightness",                     15,    3,     50, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::StepsPerRotation] =        NVSJSON_INITINT32_RNG("StepPerRot",      "How many step per rotation",            7334,    0,  20000, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::RingCalibTimeout] =        NVSJSON_INITINT32_RNG("Ring.CalToMS",    "Calibration process timeout",           40000,10000, 120000, NVSJSON_EFLAGS_None),
        [(int)Settings::Entry::RingHomeOffset] =          NVSJSON_INITINT32_RNG("Ring.HomeOffset", "Offset relative to home sensor",            0,-2000,   2000, NVSJSON_EFLAGS_None),
        [(int)Settings::Entry::RingHomeGapRange] =        NVSJSON_INITINT32_RNG("Ring.HomeGap",    "Home sensor deadband",                      0,    0,   2000, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::GateOpenedTimeout] =       NVSJSON_INITINT32_RNG("GateTimeoutS",    "Timeout (s) before the gate close",     300,   10,  38*60, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::RampOnPercent] =           NVSJSON_INITINT32_RNG("Ramp.LightOn",    "Ramp illumination ON (percent)",         30,    0,    100, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::WormholeMaxLight] =        NVSJSON_INITINT32_RNG("WH.MaxLight",     "Maximum brightness for wormhole leds. (Warning: can cause voltage drop)", 200, 0, 255, NVSJSON_EFLAGS_None),

        // Animation delay
        [(int)Settings::Entry::AnimPrelockDelayMS] =      NVSJSON_INITINT32_RNG("Dial.Anim1",      "Delay before locking the chevron (ms)", 1250, 0, 6000, NVSJSON_EFLAGS_None),
        [(int)Settings::Entry::AnimPostlockDelayMS] =     NVSJSON_INITINT32_RNG("Dial.Anim2",      "Delay after locking the chevron (ms)",  1250, 0, 6000, NVSJSON_EFLAGS_None),
        [(int)Settings::Entry::AnimPredialDelayMS] =      NVSJSON_INITINT32_RNG("Dial.Anim3",      "Delay before starting to dial (ms)",    2500, 0, 10000, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::SoundFXVolume] =           NVSJSON_INITINT32_RNG("Sound.Vol",       "Get audio player volume (percent)",     25,   0,   100, NVSJSON_EFLAGS_None),

        [(int)Settings::Entry::GateGalaxy] =              NVSJSON_INITINT32_RNG("Gate.Gal",        "Galaxy type -1: None, 0: Milky-way, 1: Pegasus, 2: Universe",      -1,   -1, (int32_t)GateGalaxy::Count-1, NVSJSON_EFLAGS_None),
    };
    NVSJSON_SHandle m_setting_handle;
    const NVSJSON_SConfig m_setting_config = { .partition_name = "nvs", .setting_entries = m_config_entries, .setting_entry_count = (uint32_t)Settings::Entry::Count };
};

