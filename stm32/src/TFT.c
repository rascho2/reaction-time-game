#include "TFT.h"
#include "Font.h"

extern SPI_HandleTypeDef hspi1;

#define TFT_CS_PORT   GPIOB
#define TFT_CS_PIN    GPIO_PIN_11
#define TFT_RST_PORT  GPIOB
#define TFT_RST_PIN   GPIO_PIN_10
#define TFT_DC_PORT   GPIOB
#define TFT_DC_PIN    GPIO_PIN_0

void TFT_Reset(void) {
    HAL_GPIO_WritePin(TFT_RST_PORT, TFT_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(TFT_RST_PORT, TFT_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(150);
}

void TFT_WriteCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_SET);
}

void TFT_WriteData(uint8_t data) {
    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_SET);
}

void TFT_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    const uint8_t *glyph = Font_GetGlyph(c);

    for (int col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (int row = 0; row < 7; row++) {
            uint16_t px = (line & (1 << row)) ? color : bg;
            TFT_FillRect(x + col * size, y + row * size, size, size, px);
        }
    }
}

void TFT_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size) {
    while (*str) {
        TFT_DrawChar(x, y, *str, color, bg, size);
        x += 6 * size;
        str++;
    }
}

void TFT_Init(void) {
    TFT_Reset();

    TFT_WriteCommand(0xEF);
    TFT_WriteData(0x03); TFT_WriteData(0x80); TFT_WriteData(0x02);

    TFT_WriteCommand(0xCF);
    TFT_WriteData(0x00); TFT_WriteData(0xC1); TFT_WriteData(0x30);

    TFT_WriteCommand(0xED);
    TFT_WriteData(0x64); TFT_WriteData(0x03); TFT_WriteData(0x12); TFT_WriteData(0x81);

    TFT_WriteCommand(0xE8);
    TFT_WriteData(0x85); TFT_WriteData(0x00); TFT_WriteData(0x78);

    TFT_WriteCommand(0xCB);
    TFT_WriteData(0x39); TFT_WriteData(0x2C); TFT_WriteData(0x00);
    TFT_WriteData(0x34); TFT_WriteData(0x02);

    TFT_WriteCommand(0xF7);
    TFT_WriteData(0x20);

    TFT_WriteCommand(0xEA);
    TFT_WriteData(0x00); TFT_WriteData(0x00);

    TFT_WriteCommand(0xC0);
    TFT_WriteData(0x23);

    TFT_WriteCommand(0xC1);
    TFT_WriteData(0x10);

    TFT_WriteCommand(0xC5);
    TFT_WriteData(0x3E); TFT_WriteData(0x28);

    TFT_WriteCommand(0xC7);
    TFT_WriteData(0x86);

    TFT_WriteCommand(0x36);
    TFT_WriteData(0x28);

    TFT_WriteCommand(0x3A);
    TFT_WriteData(0x55);

    TFT_WriteCommand(0xB1);
    TFT_WriteData(0x00); TFT_WriteData(0x18);

    TFT_WriteCommand(0xB6);
    TFT_WriteData(0x08); TFT_WriteData(0x82); TFT_WriteData(0x27);

    TFT_WriteCommand(0xF2);
    TFT_WriteData(0x00);

    TFT_WriteCommand(0x26);
    TFT_WriteData(0x01);

    TFT_WriteCommand(0xE0);
    uint8_t gammaP[] = {0x0F,0x31,0x2B,0x0C,0x0E,0x08,0x4E,0xF1,0x37,0x07,0x10,0x03,0x0E,0x09,0x00};
    for (int i = 0; i < 15; i++) TFT_WriteData(gammaP[i]);

    TFT_WriteCommand(0xE1);
    uint8_t gammaN[] = {0x00,0x0E,0x14,0x03,0x11,0x07,0x31,0xC1,0x48,0x08,0x0F,0x0C,0x31,0x36,0x0F};
    for (int i = 0; i < 15; i++) TFT_WriteData(gammaN[i]);

    TFT_WriteCommand(0x11);
    HAL_Delay(120);

    TFT_WriteCommand(0x29);
    HAL_Delay(20);
}

void TFT_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    TFT_WriteCommand(0x2A);
    TFT_WriteData(x0 >> 8); TFT_WriteData(x0 & 0xFF);
    TFT_WriteData(x1 >> 8); TFT_WriteData(x1 & 0xFF);

    TFT_WriteCommand(0x2B);
    TFT_WriteData(y0 >> 8); TFT_WriteData(y0 & 0xFF);
    TFT_WriteData(y1 >> 8); TFT_WriteData(y1 & 0xFF);

    TFT_WriteCommand(0x2C);
}

void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    TFT_SetAddressWindow(x, y, x + w - 1, y + h - 1);

    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_RESET);

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    uint32_t pixels = (uint32_t)w * h;

    for(uint32_t i = 0; i < pixels; i++) {
        while(!(SPI1->SR & SPI_SR_TXE));
        *((__IO uint8_t *)&SPI1->DR) = hi;

        while(!(SPI1->SR & SPI_SR_TXE));
        *((__IO uint8_t *)&SPI1->DR) = lo;
    }
    while(SPI1->SR & SPI_SR_BSY);

    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_SET);
}
