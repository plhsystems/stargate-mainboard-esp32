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
#include "../Common/Chevron.hpp"
#include "../Settings.hpp"
#include "../Wormhole/Wormhole.hpp"
#include "HW/PinkySGHW.hpp"

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
    enum class ECmd
    {
        Idle = 0,

        AutoHome,
        AutoCalibrate,

        KeyPress,

        DialAddress,
        ManualWormhole,
    };
    private:
    struct SCmd
    {
        ECmd eCmd;
        union
        {
            struct
            {
                uint8_t u8Symbols[9];
                uint8_t u8SymbolCount;
            } sDialAddress;
            struct
            {
                uint8_t u8Key;
            } sKeypress;
            struct
            {
                Wormhole::EType eWormholeType;
            } sManualWormhole;
        } uArg;
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

    void Init();

    void StartTask();

    // Actions
    void QueueAutoHome();
    void QueueAutoCalibrate();
    void QueueDialAddress();

    void AbortAction();

    static GateControl& getI()
    {
        static GateControl instance;
        return instance;
    }
    private:
    void PriQueueAction(SCmd cmd);

    bool AutoCalibrate();   /*!< @brief This procedure will find how many step are necessary to complete a full ring rotation. */
    bool AutoHome();        /*!< @brief Do the homing sequence, it will spin until it find it's home position. */
    bool DialAddress();

    void AnimRampLight(bool bIsActive);
    // Stepper
    bool SpinUntil(ESpinDirection eSpinDirection, ETransition eTransition, uint32_t u32TimeoutMS, int32_t* ps32refTickCount);
    bool MoveStepperTo(int32_t s32Ticks);

    private:
    static void TaskRunning(void* pArg);
    static void tmr_signal_callback(void* arg);

    TaskHandle_t m_sGateControlHandle;

    // Actions
    volatile bool m_bIsCancelAction;
    SCmd m_sCmd;

    int32_t m_bIsHomingDone = false;
    int32_t m_s32CurrentPositionTicks = 0;

    Stepper m_stepper;
};