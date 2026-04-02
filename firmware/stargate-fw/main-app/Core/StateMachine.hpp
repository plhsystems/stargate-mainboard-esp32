#pragma once

#include <cstdint>
#include <cstddef>
#include "esp_log.h"

/**
 * @brief Gate controller state machine states
 *
 * Defines all possible states the gate controller can be in.
 * State transitions are validated to ensure only valid transitions occur.
 */
enum class GateState : uint8_t
{
    Idle = 0,           ///< Gate is idle, ready for commands
    Homing,             ///< Auto-homing sequence in progress
    Calibrating,        ///< Auto-calibration sequence in progress
    Dialing,            ///< Dialing a gate address
    WormholeOpen,       ///< Wormhole is active (kawoosh complete)
    ManualWormhole,     ///< Manual wormhole mode (no dial sequence)
    Error,              ///< Error state, requires reset or new command
    Aborting,           ///< Aborting current operation

    Count
};

/**
 * @brief Events that trigger state transitions
 */
enum class GateEvent : uint8_t
{
    // User commands
    CmdHome = 0,        ///< User requested auto-home
    CmdCalibrate,       ///< User requested auto-calibrate
    CmdDial,            ///< User requested to dial an address
    CmdManualWormhole,  ///< User requested manual wormhole
    CmdAbort,           ///< User requested abort

    // Internal events
    OperationComplete,  ///< Current operation completed successfully
    OperationFailed,    ///< Current operation failed
    Timeout,            ///< Operation timed out
    WormholeTimeout,    ///< Wormhole max duration reached
    Reset,              ///< Reset from error state

    Count
};

/**
 * @brief Get human-readable string for a state
 */
inline const char* GateStateToString(GateState state)
{
    switch (state)
    {
        case GateState::Idle:           return "Idle";
        case GateState::Homing:         return "Homing";
        case GateState::Calibrating:    return "Calibrating";
        case GateState::Dialing:        return "Dialing";
        case GateState::WormholeOpen:   return "Wormhole Open";
        case GateState::ManualWormhole: return "Manual Wormhole";
        case GateState::Error:          return "Error";
        case GateState::Aborting:       return "Aborting";
        default:                        return "Unknown";
    }
}

/**
 * @brief Get human-readable string for an event
 */
inline const char* GateEventToString(GateEvent event)
{
    switch (event)
    {
        case GateEvent::CmdHome:            return "CmdHome";
        case GateEvent::CmdCalibrate:       return "CmdCalibrate";
        case GateEvent::CmdDial:            return "CmdDial";
        case GateEvent::CmdManualWormhole:  return "CmdManualWormhole";
        case GateEvent::CmdAbort:           return "CmdAbort";
        case GateEvent::OperationComplete:  return "OperationComplete";
        case GateEvent::OperationFailed:    return "OperationFailed";
        case GateEvent::Timeout:            return "Timeout";
        case GateEvent::WormholeTimeout:    return "WormholeTimeout";
        case GateEvent::Reset:              return "Reset";
        default:                            return "Unknown";
    }
}

/**
 * @brief State transition definition
 */
struct StateTransition
{
    GateState from;
    GateEvent event;
    GateState to;
};

/**
 * @brief State machine for gate controller
 *
 * Manages state transitions and validates that only legal transitions occur.
 * This provides a formal, testable model of the gate controller behavior.
 */
class GateStateMachine
{
public:
    GateStateMachine() : m_current_state(GateState::Idle) {}

    /**
     * @brief Process an event and transition to new state if valid
     * @param event The event to process
     * @return true if transition was valid and occurred, false otherwise
     */
    bool processEvent(GateEvent event)
    {
        GateState new_state = getNextState(m_current_state, event);

        if (new_state == m_current_state && !isValidSelfTransition(event))
        {
            // Invalid transition
            ESP_LOGW("StateMachine", "Invalid transition: %s + %s",
                     GateStateToString(m_current_state),
                     GateEventToString(event));
            return false;
        }

        ESP_LOGI("StateMachine", "Transition: %s -> %s (event: %s)",
                 GateStateToString(m_current_state),
                 GateStateToString(new_state),
                 GateEventToString(event));

        m_current_state = new_state;
        return true;
    }

    /**
     * @brief Get current state
     */
    GateState getState() const { return m_current_state; }

    /**
     * @brief Check if currently in idle state
     */
    bool isIdle() const { return m_current_state == GateState::Idle; }

    /**
     * @brief Check if currently in error state
     */
    bool isError() const { return m_current_state == GateState::Error; }

    /**
     * @brief Check if a wormhole is currently active
     */
    bool isWormholeActive() const
    {
        return m_current_state == GateState::WormholeOpen ||
               m_current_state == GateState::ManualWormhole;
    }

    /**
     * @brief Check if gate is busy (not idle and not in error)
     */
    bool isBusy() const
    {
        return m_current_state != GateState::Idle &&
               m_current_state != GateState::Error;
    }

    /**
     * @brief Force state (use with caution, bypasses validation)
     */
    void forceState(GateState state)
    {
        ESP_LOGW("StateMachine", "Forcing state to: %s", GateStateToString(state));
        m_current_state = state;
    }

private:
    GateState m_current_state;

    /**
     * @brief State transition table
     *
     * Defines all valid state transitions. Any transition not in this table
     * is considered invalid and will be rejected.
     */
    static GateState getNextState(GateState current, GateEvent event)
    {
        // clang-format off
        static const StateTransition transitions[] = {
            // From Idle state
            { GateState::Idle,           GateEvent::CmdHome,            GateState::Homing },
            { GateState::Idle,           GateEvent::CmdCalibrate,       GateState::Calibrating },
            { GateState::Idle,           GateEvent::CmdDial,            GateState::Dialing },
            { GateState::Idle,           GateEvent::CmdManualWormhole,  GateState::ManualWormhole },

            // From Homing state
            { GateState::Homing,         GateEvent::OperationComplete,  GateState::Idle },
            { GateState::Homing,         GateEvent::OperationFailed,    GateState::Error },
            { GateState::Homing,         GateEvent::Timeout,            GateState::Error },
            { GateState::Homing,         GateEvent::CmdAbort,           GateState::Aborting },

            // From Calibrating state
            { GateState::Calibrating,    GateEvent::OperationComplete,  GateState::Idle },
            { GateState::Calibrating,    GateEvent::OperationFailed,    GateState::Error },
            { GateState::Calibrating,    GateEvent::Timeout,            GateState::Error },
            { GateState::Calibrating,    GateEvent::CmdAbort,           GateState::Aborting },

            // From Dialing state
            { GateState::Dialing,        GateEvent::OperationComplete,  GateState::WormholeOpen },
            { GateState::Dialing,        GateEvent::OperationFailed,    GateState::Error },
            { GateState::Dialing,        GateEvent::Timeout,            GateState::Error },
            { GateState::Dialing,        GateEvent::CmdAbort,           GateState::Aborting },

            // From WormholeOpen state
            { GateState::WormholeOpen,   GateEvent::CmdAbort,           GateState::Aborting },
            { GateState::WormholeOpen,   GateEvent::WormholeTimeout,    GateState::Idle },
            { GateState::WormholeOpen,   GateEvent::OperationComplete,  GateState::Idle },

            // From ManualWormhole state
            { GateState::ManualWormhole, GateEvent::CmdAbort,           GateState::Aborting },
            { GateState::ManualWormhole, GateEvent::OperationComplete,  GateState::Idle },

            // From Aborting state
            { GateState::Aborting,       GateEvent::OperationComplete,  GateState::Idle },
            { GateState::Aborting,       GateEvent::OperationFailed,    GateState::Error },

            // From Error state
            { GateState::Error,          GateEvent::Reset,              GateState::Idle },
            { GateState::Error,          GateEvent::CmdHome,            GateState::Homing },
            { GateState::Error,          GateEvent::CmdCalibrate,       GateState::Calibrating },
        };
        // clang-format on

        for (size_t i = 0; i < sizeof(transitions) / sizeof(transitions[0]); ++i)
        {
            if (transitions[i].from == current && transitions[i].event == event)
            {
                return transitions[i].to;
            }
        }

        // No valid transition found, return current state
        return current;
    }

    /**
     * @brief Check if staying in same state is valid for this event
     */
    static bool isValidSelfTransition(GateEvent event)
    {
        // Some events are valid even if they don't cause a state change
        // (e.g., CmdAbort when already aborting)
        return event == GateEvent::CmdAbort;
    }
};
