#pragma once

#include <stdint.h>
#include <array>
#include <vector>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "misc-macro.h"
#include "../Gate/BaseGate.hpp"
#include "../Gate/GateFactory.hpp"
#include "../Gate/GateAddress.hpp"
#include "Chevron.hpp"
#include "../Settings.hpp"
#include "../Wormhole/Wormhole.hpp"
#include "../HW/SGHW_HAL.hpp"

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
        ECmd cmd;

        char status_text[ERROR_LEN+1];

        bool has_last_error;
        char last_error[ERROR_LEN+1];

        bool is_cancel_requested;
    };

    private:

    struct SDialArg
    {
        GateAddress gate_address;
        Wormhole::EType wormhole_type;
    };

    struct SManualWormholeArg
    {
        Wormhole::EType wormhole_type;
    };

    struct SCmd
    {
        ECmd cmd;
        SDialArg dial_address;
        struct
        {
            uint8_t key;
        } keypress;
        SManualWormholeArg manual_wormhole;
    };

    GateControl();

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

    bool AutoCalibrate(const char** error_msg);   /*!< @brief This procedure will find how many step are necessary to complete a full ring rotation. */
    bool AutoHome(const char** error_msg);        /*!< @brief Do the homing sequence, it will spin until it find it's home position. */
    bool DialAddress(const SDialArg& dial_arg, const char** error_msg);

    void AnimRampLight(bool bIsActive);

    void LockClamp();
    void ReleaseClamp();
    private:
    static void TaskRunning(void* pArg);

    TaskHandle_t m_gate_control_handle;

    // Control homing and encoder.
    StaticSemaphore_t m_semaphore_create_mutex;
    SemaphoreHandle_t m_semaphore_handle;

    // Actions
    volatile bool m_is_cancel_action;
    SCmd m_curr_cmd;
    SCmd m_next_cmd;

    // Error management
    char m_errors[ERROR_LEN+1] = {0};
    bool m_is_in_error;

    // Position
    int32_t m_is_homing_done = false;
    int32_t m_current_position_ticks = 0;

    SGHW_HAL* m_sghw_hal;

    inline static const char* m_texts[] =
    {
        [(int)ECmd::Idle]           = "Idle",
        [(int)ECmd::Stop]           = "Stopping",
        [(int)ECmd::AutoHome]       = "Auto homing",
        [(int)ECmd::AutoCalibrate]  = "Auto calibrating",
        [(int)ECmd::KeyPress]       = "Keypress",
        [(int)ECmd::DialAddress]    = "Dialing",
        [(int)ECmd::ManualWormhole] = "Manual wormhole",
    };
    static_assert((int)ECmd::Count == (sizeof(m_texts)/sizeof(m_texts[0])), "Command text missmatch");

    public:
    inline static const char* GetCmdText(ECmd eCmd) { return m_texts[(int)eCmd]; }
};