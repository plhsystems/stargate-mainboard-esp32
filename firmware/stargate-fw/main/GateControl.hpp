#pragma once

#include <stdint.h>
#include <array>
#include <vector>
#include <string>
#include "GateType.hpp"

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
};