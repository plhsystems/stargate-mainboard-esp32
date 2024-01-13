#pragma once

#include <stdint.h>
#include <array>
#include <vector>
#include <string>
#include "Gate/BaseGate.hpp"
#include "Gate/GateFactory.hpp"

class GateControl
{
    enum class ECmd
    {
        Idle,
        GoHome,
        Aborted,
        Dial,
        AutoCalibrate,

        ManualWormhole,
    };

    public:
    // Constructor
    GateControl();

    void Init();

    void StartTask();

    // Actions
    void QueueAction();

    private:
    static void TaskRunning(GateControl* pc);
};