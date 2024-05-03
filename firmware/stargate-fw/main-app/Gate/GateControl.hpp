#pragma once

#include <stdint.h>
#include <array>
#include <vector>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "misc-macro.h"
#include "../Gate/BaseGate.hpp"
#include "../Gate/GateFactory.hpp"
#include "../Gate/GateAddress.hpp"
#include "Chevron.hpp"
#include "../Settings.hpp"
#include "../Wormhole/Wormhole.hpp"
#include "../HW/SGHW_HAL.hpp"

enum class ETransition
{
    Rising,
    Failing
};

enum class ESpinDirection
{
    CCW,
    CW
};

class GateControl
{
    public:
    static constexpr uint32_t ERROR_LEN = 128;

    enum class ECmd
    {
        Idle = 0,

        Stop,

        AutoHome,
        AutoCalibrate,

        KeyPress,

        DialAddress,
        ManualWormhole,

        Count
    };

    struct UIState
    {
        ECmd eCmd;

        char szStatusText[ERROR_LEN+1];

        bool bHasLastError;
        char szLastError[ERROR_LEN+1];

        bool bIsCancelRequested;
    };

    private:

    struct SDialArg
    {
        GateAddress sGateAddress;
        Wormhole::EType eWormholeType;
    };

    struct SManualWormholeArg
    {
        Wormhole::EType eWormholeType;
    };

    struct SCmd
    {
        ECmd eCmd;
        SDialArg sDialAddress;
        struct
        {
            uint8_t u8Key;
        } sKeypress;
        SManualWormholeArg sManualWormhole;
    };

    GateControl();

    #define STEPEND_BIT    0x01
    struct Stepper
    {
        esp_timer_handle_t sSignalTimerHandle;
        TaskHandle_t sTskControlHandle;

        int32_t s32Period = 0;
        // Counter
        bool bIsCCW;
        int32_t s32Count = 0;
        int32_t s32Target = 0;
    };

    public:
    // Singleton pattern
    GateControl(GateControl const&) = delete;
    void operator=(GateControl const&) = delete;

    void Init(SGHW_HAL* pSGHWHal);

    void StartTask();

    // Actions
    void QueueAutoHome();
    void QueueAutoCalibrate();
    void QueueDialAddress(GateAddress& ga);
    void QueueManualWormhole(Wormhole::EType type);

    void AbortAction();

    void GetState(UIState& uiState);

    static GateControl& getI()
    {
        static GateControl instance;
        return instance;
    }
    private:
    void PriQueueAction(SCmd cmd);

    void AutoCalibrate();   /*!< @brief This procedure will find how many step are necessary to complete a full ring rotation. */
    void AutoHome();        /*!< @brief Do the homing sequence, it will spin until it find it's home position. */
    void DialAddress(const SDialArg& sDialArg);

    void AnimRampLight(bool bIsActive);
    // Stepper
    void SpinUntil(ESpinDirection eSpinDirection, ETransition eTransition, uint32_t u32TimeoutMS, int32_t* ps32refTickCount);
    void MoveStepperTo(int32_t s32Ticks);

    private:
    static void TaskRunning(void* pArg);
    static void tmr_signal_callback(void* arg);

    TaskHandle_t m_sGateControlHandle;

    // Control homing and encoder.
    StaticSemaphore_t m_xSemaphoreCreateMutex;
    SemaphoreHandle_t m_xSemaphoreHandle;

    // Actions
    volatile bool m_bIsCancelAction;
    SCmd m_sCurrCmd;
    SCmd m_sNextCmd;

    // Error management
    char m_szErrors[ERROR_LEN+1] = {0};
    bool m_bIsInError;

    // Position
    int32_t m_bIsHomingDone = false;
    int32_t m_s32CurrentPositionTicks = 0;

    SGHW_HAL* m_pSGHWHAL;

    Stepper m_stepper;

    inline static const char* m_szTexts[] =
    {
        [(int)ECmd::Idle]           = "Idle",
        [(int)ECmd::Stop]           = "Stopping",
        [(int)ECmd::AutoHome]       = "Auto homing",
        [(int)ECmd::AutoCalibrate]  = "Auto calibrating",
        [(int)ECmd::KeyPress]       = "Keypress",
        [(int)ECmd::DialAddress]    = "Dialing",
        [(int)ECmd::ManualWormhole] = "Manual wormhole",
    };
    static_assert((int)ECmd::Count == (sizeof(m_szTexts)/sizeof(m_szTexts[0])), "Command text missmatch");

    public:
    inline static const char* GetCmdText(ECmd eCmd) { return m_szTexts[(int)eCmd]; }
};