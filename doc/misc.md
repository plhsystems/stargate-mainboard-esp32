# Miscellaneous things

## LED linearity

LED lumen output is somewhat linear according to the pwm duty cycle. 
However, human eyes perception isn't linear (I don't know about the Asguard). 

If we want to do a fade-in / fade-out using a for loop increasing linearly the duty cycle, it will works poorly. 

Recommended formula to give a perceived linear effect to human eyes (sorry Thor).

| Variable | Description |
|---|---|
| input |Input between [0-1]
| output |Output between [0-1]

$$ out={(10^{input}-1) \over 9} $$

## FreeRTOS Tick Rate and Timing

### Tick Resolution

The system uses a 1000Hz FreeRTOS tick rate (`CONFIG_FREERTOS_HZ=1000`):

**Timing Resolution**:
- **Tick period**: 1ms (1000 ticks per second)
- **Minimum delay**: `vTaskDelay(pdMS_TO_TICKS(1))` = 1ms
- **Typical delays**: Multiples of 1ms provide precise timing

**Recent Change**: Upgraded from 100Hz (10ms resolution) to 1000Hz for improved timing accuracy in:
- Stepper motor control (600-1600µs per step with dynamic frequency)
- Wormhole LED refresh (40ms = exactly 40 ticks)
- BLE heartbeat (1000ms = exactly 1000 ticks)
- Servo PWM timing (20ms = exactly 20 ticks)

**Implications**:
- **pdMS_TO_TICKS() accuracy**: Converts milliseconds to ticks with 1ms precision
  ```cpp
  vTaskDelay(pdMS_TO_TICKS(500));  // Exactly 500 ticks = 500ms
  ```
- **CPU overhead**: Higher tick rate increases context switch frequency but provides better responsiveness
- **Timer accuracy**: ESP Timer ISR dispatch remains independent at microsecond resolution

**Task Scheduling**:
- All tasks run at `tskIDLE_PRIORITY` (cooperative scheduling)
- Tasks yield via delays and blocking calls
- 1ms tick resolution allows fine-grained task timing

## Embedded Web Assets Generation

The web interface files (HTML, CSS, JavaScript, images) are embedded directly into the firmware binary to be served by the ESP32's HTTP server. This eliminates the need for an SD card or external file system for the web UI.

### Process

The `embeddedgen.py` tool converts web assets into C arrays. It is invoked automatically by CMake at build time — no manual step needed.

To run it manually:

```bash
cd firmware/stargate-fw
python3 ../tools/embeddedgen.py -i "./main-app/webserver-assets" -o "./main-app/WebServer" --compress-all
```

The `--compress-all` flag gzip-compresses all text-based assets (HTML, CSS, JS, SVG, JSON, etc.) automatically. The HTTP server sets `Content-Encoding: gzip` when serving compressed files so browsers decompress them transparently.

**Generated Files**:
- `WebServer/EmbeddedFiles.c` - C array definitions with file contents (may be gzip-compressed)
- `WebServer/EmbeddedFiles.h` - Header with extern declarations and `EF_SFile` struct
- `WebServer/EmbeddedFiles.bin` - Binary concatenation of all files
- `WebServer/EmbeddedFiles.txt` - Text listing of all embedded files

**`EF_SFile` struct fields**:
- `filename` - relative path string
- `length` - original (decompressed) file size in bytes
- `compressed_length` - bytes sent over the wire (equals `length` when not compressed)
- `flags` - `EF_EFLAGS_GZip` if compressed, `EF_EFLAGS_None` otherwise
- `start_addr` - pointer into the `EF_g_blobs[]` array

**When to Regenerate**:
- After modifying any HTML, CSS, or JavaScript files
- After adding/removing web assets
- CMake handles this automatically — a full build always regenerates if sources changed

**Integration**:
`WebServer.cpp` looks up files by path in `EF_g_files[]`, sets `Content-Encoding: gzip` if compressed, and sends exactly `compressed_length` bytes from `start_addr`.

**Size Considerations**:
- gzip compression reduces JS/CSS/HTML size by ~60–70%
- Monitor app partition size; 1900 K OTA partitions leave comfortable headroom after compression
