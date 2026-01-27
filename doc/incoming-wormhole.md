# Incoming Wormhole

## Overview

An incoming wormhole occurs when a remote Stargate dials your gate's address. The receiving gate must display the incoming sequence differently based on the gate type. This document describes the visual and mechanical behavior for each Stargate variant.

---

## SG1/Movie Gate

### Visual Behavior
Each chevron lights up in succession as the remote gate encodes each symbol in the address.

**Sequence** (for 7-chevron address):
1. Chevron 1 (right of master) lights up when first symbol is encoded
2. Chevron 2 lights up when second symbol is encoded
3. ... continues clockwise
4. Chevron 7 (master chevron) lights up when final symbol (point of origin) is encoded
5. All chevrons flash when wormhole establishes

### Ring Behavior
**Movie Version**:
- Ring remains stationary
- No physical movement during incoming sequence
- Only chevron LEDs are activated

**SG1 Version**:
- Ring remains stationary
- Chevron LEDs light in sequence
- Optional: Symbols can be illuminated on the ring at their locked positions

### Sound Effects
- Chevron activation sound (per chevron)
- Wormhole establishment sound (kawoosh)
- Ambient wormhole hum

---

## Atlantis/Pegasus Gate

### Visual Behavior
Combines chevron sequence with animated symbol progression on the ring.

**Sequence**:
1. Each chevron lights up in succession (like SG1)
2. **Simultaneously**: Symbols light up in a clockwise pattern around the ring
3. The symbol progression acts as a "virtual spin" showing dial progress
4. Progression starts from the master chevron position
5. Speed: One complete revolution per encoded symbol

### Ring Behavior
- Ring remains stationary (no physical movement)
- All 36 ring segments are LED-backlit screens
- Symbols light up sequentially in clockwise direction
- Creates illusion of spinning dial
- Each symbol encoding causes one full "virtual rotation"

### Sound Effects
- Atlantis-specific chevron activation
- "Virtual spin" whoosh sound during symbol progression
- Atlantis wormhole establishment

---

## Universe Gate

### Visual Behavior
Completely different mechanism - no chevron locking, entire gate rotates.

**Sequence**:
1. Entire gate body spins (counter-clockwise when viewed from front)
2. Gate decelerates as symbol approaches alignment with top position
3. Symbol locks in place (gate stops briefly)
4. Symbol illuminates on the gate surface
5. Gate accelerates and continues to next symbol
6. No physical chevron movement
7. All 9 chevrons are always illuminated (blue glow)

### Ring Behavior
- **Physical rotation**: Stepper motor actively rotates the ring
- Symbol count: 36 symbols (not equally spaced!)
- Alignment: Symbols align with top center position (12 o'clock)
- Speed: Variable speed (fast between symbols, slow during lock)

### Symbol Spacing
**Critical**: Universe gate symbols are NOT equally spaced!

---

## Comparison Table

| Feature | SG1/Movie | Atlantis | Universe |
|---------|-----------|----------|----------|
| Ring Movement | None | None | Full rotation |
| Chevron Activation | Sequential | Sequential | All lit (static) |
| Symbol Display | Optional | Clockwise progression | Physical alignment |
| Symbol Count | 39 | 36 | 36 |
| Symbol Spacing | Equal | Equal | Unequal |
| Duration per Symbol | ~2 seconds | ~2 seconds | ~3-4 seconds |
| Sound | Classic locks | Atlantis whoosh | Rotation hum |
| Mechanical Complexity | Low | Low | High |

---

## Related Documentation

- [Architecture](architecture.md) - Overall system design
- [Dial Algorithm](dial-algorithm.md) - Outgoing wormhole mechanics
- [Stargate Types](stargate-types.md) - Detailed gate specifications
- [BLE Protocol](ble-protocol.md) - Ring communication
- [API Reference](api-reference.md) - HTTP API for triggering incoming wormholes
