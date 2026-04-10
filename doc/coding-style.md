# Coding Style Guide

This document outlines the coding style for the `oe-heatgrip-gdo` projects.

## Variable Naming

- **Local variables**: Use `snake_case`.
- **Global variables**: Use `g_snake_case`.
- **Member variables**: Use `m_snake_case`.
- **Functions**: Use `lowerPascalCase` (camelCase).
- **Constants and Macros**: Use `UPPER_CASE`.
- **Classes/Structs**: Use `PascalCase` (e.g., `struct MessagePacket;`).
- **Enums**: Use `enum class` (scoped enums) with `PascalCase` members (e.g., `enum class DeviceState { ... };`).
- **Acronyms**: Keep acronyms in uppercase (e.g., `OTA`, `GDO`, `PMK`, `SSID`, `WAP`, `PWM`, `BLE`, `STA`, `AP`).
- **Brackets**: The starting brace `{` **MUST** be on a new line (Allman style).

### Type Prefixes (Hungarian Notation)

> [!IMPORTANT]
> **DO NOT** use prefixes linked to the type in variable names.
>
> - **Incorrect**: `bool b_is_active`, `uint32_t u32_count`, `char* sz_name`, `void* p_data`.
> - **Correct**: `bool is_active`, `uint32_t count`, `char* name`, `void* data`.

## Pointers

- **Pointer star placement**: The `*` belongs to the type, not the variable name.
  - **Correct**: `char* name`, `void* data`, `const char* str`.
  - **Incorrect**: `char *name`, `void *data`, `const char *str`.

## const correctness

- **Local variables**: Always declare local variables `const` if they are never modified after initialization.
  - **Correct**: `const int32_t count = getCount();`
  - **Incorrect**: `int32_t count = getCount();` *(when `count` is never reassigned)*

## Conditions

- **Yoda conditions**: Place the constant on the left side of comparisons to prevent accidental assignment.
  - **Correct**: `if (0 == getValue())`, `if (ESP_OK == esp_init())`.
  - **Incorrect**: `if (getValue() == 0)`, `if (esp_init() == ESP_OK)`.

## Project Structure

- Source files should use the `.cpp` extension.
- Namespaces should be used to group related functions and variables (e.g., `HardwareGPIO`, `Settings`, `WebServer`, `MemBlock`).