#pragma once

#include <stdint.h>
#include <array>
#include <vector>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "../Gate/BaseGate.hpp"
#include "../Gate/GateFactory.hpp"
#include "../Common/Chevron.hpp"
#include "../Settings.hpp"

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

        Abort,

        AutoHome,
        AutoCalibrate,

        KeyPressSymbol,
        KeyPressCommit,

        DialAddress,
        ManualWormhole,
    };

    public:
    // Constructor
    GateControl();

    void Init();

    void StartTask();

    // Actions
    void QueueAction(ECmd cmd);
    void AbortAction();

    private:
    bool AutoCalibrate();   /*!< @brief This procedure will find how many step are necessary to complete a full ring rotation. */
    bool AutoHome();        /*!< @brief Do the homing sequence, it will spin until it find it's home position. */

    bool SpinUntil(ESpinDirection eSpinDirection, ETransition eTransition, uint32_t u32TimeoutMS, int32_t* ps32refTickCount);

    private:
    static void TaskRunning(void* pArg);

    TaskHandle_t m_sGateControlHandle;

    // Actions
    volatile bool m_bIsCancelAction;
    volatile ECmd m_eCmd;
};