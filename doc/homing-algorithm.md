# Homing algorithm

The homing algorithm should run every time the gate power up.
It's also possible to manually run it.

## Hall sensor

Also hall sensor have an hysterisis, meaning the position to activate it and deactivate it are not the same.

There could be several degrees of hysterisis, depending on how it is mechanically assembled or what sensor is used.

This diagram represent an exagerated version of the phenomena.

![](./assets/hysterisis-example.png)

## Light sensor

The ring contains an hidden hole with a led on one side and a sensor on the other side of the hole.
The hysterisis is likely very low. But the algorithm remains the same as for the Hall sensor.

# Homing procedure

If the ring is already in a position were the home sensor is ON (magnet near the sensor), it doesn't mean the homing is done correctly.

The stepper while unpowered could move the ring slightly out of alignment.

But the good part is that it makes the homing significantly faster, it doesn't need the ring to do an entire rotation.

- Spin the ring until the home sensor is ON. Consider it's the temporary 0 position.
- Continue moving in the same direction until the home sensor is OFF.
- Move in the other direction until the home sensor is ON. (step count should decrement)
- Check how many step were done, divide the value by two to find the middle.
- Move the gate by the number of step the previous part of the algorithm returned to center it.

If the home sensor sensor is off at the start of the process, it will takes more time because it may needs to do an entire rotation to find the home point.

## Error Handling and Timeouts

### Timeout Behavior

The homing and calibration operations have built-in timeout protection:

**AutoHome** (GateControl.cpp:247-310):
- **Timeout**: 30 seconds (30000ms)
- **Returns**: `false` on timeout, `true` on success
- **Failure Condition**: If home sensor transition not detected within timeout

**AutoCalibrate** (GateControl.cpp:188-245):
- **Timeout**: 30 seconds (30000ms) per rotation detection
- **Returns**: `false` on timeout, `true` on success
- **Requires**: Multiple sensor transitions (Rising → Rising, Failing → Rising)
- **Stores**: `StepsPerRotation` and `RingHomeGapRange` in NVS on success

### Implementation Details

**SpinUntil Function Signature**:
```cpp
bool SpinUntil(ESpinDirection spin_direction,
               ETransition transition,
               uint32_t timeout_ms,
               int32_t* ref_tick_count);
```

**Returns**:
- `true`: Sensor transition detected successfully
- `false`: Timeout reached without detecting transition

**Error Propagation**:
```cpp
// AutoHome checks each SpinUntil return value
if (!m_sghw_hal->SpinUntil(ESpinDirection::CW, ETransition::Failing, timeout, nullptr)) {
    break;  // Abort homing sequence
}
if (!m_sghw_hal->SpinUntil(ESpinDirection::CCW, ETransition::Rising, timeout, nullptr)) {
    break;  // Abort homing sequence
}
```

### Why Timeouts Matter

**Protection Against**:
- **Mechanical failures**: Stepper motor disconnected or jammed
- **Sensor failures**: Hall sensor not working or misaligned
- **Electrical issues**: Loose connections or power problems
- **Infinite loops**: Prevents system hang if hardware malfunctions

**User Impact**:
- Homing failure returns control to user immediately
- Error can be displayed in web interface
- User can investigate hardware issues and retry
- System remains responsive even with hardware problems

### Fast vs Slow Path

**Fast Path** (sensor already active):
1. Exit home zone clockwise (Failing transition)
2. Re-enter home zone counter-clockwise (Rising transition)
3. Center in deadband
4. **Total time**: ~5-10 seconds (depends on ring size)

**Slow Path** (sensor inactive):
1. Search for home zone counter-clockwise (Rising transition)
2. Could require full rotation
3. Center in deadband
4. **Total time**: ~20-30 seconds (full rotation)

**Both paths** respect the 30-second timeout for each SpinUntil operation.
