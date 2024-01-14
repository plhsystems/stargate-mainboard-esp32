#pragma once

#include <stdint.h>
#include <array>
#include <vector>
#include <string>
#include "Gate/BaseGate.hpp"
#include "Gate/GateFactory.hpp"
#include "Common/Chevron.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

class GateControl
{
    enum class ECmd
    {
        Idle = 0,

        Abort,

        GoHome,
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
    void QueueAction();
    void AbortAction();

    private:
    static void TaskRunning(void* pArg);

    TaskHandle_t m_sGateControlHandle;
    volatile bool m_bIsCancelAction;
};