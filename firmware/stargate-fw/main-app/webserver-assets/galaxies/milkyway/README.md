# Stargate Control Interface

A pure HTML/CSS/JS recreation of the Stargate DHD control interface from the TV show.

## Features

- **Authentic Stargate Ring**: Rotating outer ring with 39 address symbols, metallic sheen, and glowing cyan effects
- **Detailed Chevrons**: 7 three-dimensional wedge-shaped chevrons with:
  - Metallic frame with realistic lighting and shadows
  - Glowing red/orange center lights when active
  - Dramatic pulsing effects with brightness changes
  - Small indicator dots at the base
- **Pulsing Event Horizon**: Multi-layered orange energy vortex in the center
- **Mechanical Ring Structure**: Visible grooves, segments, and multiple ring layers showing gate mechanics
- **Lock Indicators**: Four cyan locking beams that extend from the gate center
- **Live Status Displays**:
  - Binary data stream
  - Energy meter with fluctuating levels
  - Chevron lock indicators
  - Glyph displays
  - Authorization codes
- **Dynamic Effects**:
  - Scan lines
  - Text flickering
  - Pulsing glows
  - Rotating elements

## Usage

Simply open `stargate.html` in any modern web browser. No server required!

```bash
# From the terminal:
open stargate.html

# Or just double-click the file
```

## Keyboard Controls

- **SPACE**: Cycle through engagement phases (DIALING → ENCODING → ENGAGED → WORMHOLE STABLE)
- **C**: Activate all 7 chevrons
- **R**: Reset chevrons to default pattern

## Technical Details

- Pure CSS animations (no images)
- Responsive design
- Color scheme: Cyan (#00d9ff), Orange (#ff8800), Red (#ff0000)
- Matches SGC control room aesthetic from SG-1
- 39 address symbols positioned around rotating ring (matching actual gate)
- Layered ring structure with metallic gradients and depth
- Three-dimensional chevrons with:
  - 9-point polygon clip-path for authentic wedge shape
  - Dual pseudo-elements (::before for frame, ::after for center light)
  - Metallic gradients with inset highlights and shadows
  - Radial gradient center lights with pulsing animations
  - Small indicator dots that glow cyan when active
- Multi-layered event horizon with radial gradients

## Files

- `stargate.html` - Main interface structure
- `stargate.css` - All styling and animations
- `stargate.js` - Interactive elements and dynamic updates

## Animations

- **Ring Rotation**: 20s continuous rotation with all 39 symbols
- **Ring Shimmer**: 8s metallic sheen effect for depth
- **Event Horizon**: 2s pulsing cycle with multi-layer glow
- **Chevron Glow**: 1.5s dramatic breathing effect on chevron frame
- **Chevron Light Pulse**: 1.5s pulsing and scaling effect on center lights
- **Lock Indicators**: Pulsing cyan beams with center dots
- **Energy Meter**: 3s fluctuation
- **Binary Display**: Updates every 2s
- **Scan Lines**: Random appearance every 5s
- **Glyph Cycling**: Symbols change every 5s
