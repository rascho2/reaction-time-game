#ifndef TFT_H
#define TFT_H

#include "main.h"

void TFT_Reset(void);
void TFT_Init(void);
void TFT_WriteCommand(uint8_t cmd);
void TFT_WriteData(uint8_t data);
void TFT_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void TFT_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
void TFT_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size);
#endif
