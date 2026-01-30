#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void GPIO_Init();

void GPIO_EnableHoldPowerPin(bool enabled);

void GPIO_SetPixel(uint32_t index, uint8_t red, uint8_t green, uint8_t blue);
void GPIO_ClearAllPixels();
void GPIO_RefreshPixels();

#ifdef __cplusplus
}
#endif

#endif