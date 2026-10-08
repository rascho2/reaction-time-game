#include "main.h"
#include "TFT.h"
#include <string.h>

SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart5;

#define TFT_CS_PORT   GPIOB
#define TFT_CS_PIN    GPIO_PIN_11
#define TFT_RST_PORT  GPIOB
#define TFT_RST_PIN   GPIO_PIN_10
#define TFT_DC_PORT   GPIOB
#define TFT_DC_PIN    GPIO_PIN_0
#define TFT_BL_PORT   GPIOB
#define TFT_BL_PIN    GPIO_PIN_1
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_YELLOW  0xFFE0
#define COLOR_GREEN   0x07E0

char p1_name[4] = "AAA";
char p2_name[4] = "BBB";
uint8_t nameCursor = 0;

#define LEADERBOARD_SIZE 5

typedef struct {
    char name[4];
    uint32_t time_ms;
} PlayerResult;

PlayerResult leaderboard[LEADERBOARD_SIZE];
uint8_t currentLeaderboardEntries = 0;

uint32_t playerTimes[2];
uint8_t playerFinished[2];

void IntToStr(uint32_t val, char* str) {
    if (val == 0) { str[0] = '0'; str[1] = '\0'; return; }
    char temp[10]; int i = 0;
    while(val > 0) { temp[i++] = (val % 10) + '0'; val /= 10; }
    int j = 0;
    while(i > 0) { str[j++] = temp[--i]; }
    str[j] = '\0';
}

void AddLeaderboardEntry(char* name, uint32_t time) {
    if (currentLeaderboardEntries < LEADERBOARD_SIZE) {
        for(int i=0; i<3; i++) leaderboard[currentLeaderboardEntries].name[i] = name[i];
        leaderboard[currentLeaderboardEntries].name[3] = '\0';
        leaderboard[currentLeaderboardEntries].time_ms = time;
        currentLeaderboardEntries++;
    } else {
        if (time < leaderboard[LEADERBOARD_SIZE - 1].time_ms) {
            for(int i=0; i<3; i++) leaderboard[LEADERBOARD_SIZE - 1].name[i] = name[i];
            leaderboard[LEADERBOARD_SIZE - 1].name[3] = '\0';
            leaderboard[LEADERBOARD_SIZE - 1].time_ms = time;
        } else return;
    }
    for (int i = 0; i < currentLeaderboardEntries - 1; i++) {
        for (int j = 0; j < currentLeaderboardEntries - i - 1; j++) {
            if (leaderboard[j].time_ms > leaderboard[j + 1].time_ms) {
                PlayerResult temp = leaderboard[j];
                leaderboard[j] = leaderboard[j + 1];
                leaderboard[j + 1] = temp;
            }
        }
    }
}

#define JOY_UP_PORT     GPIOC
#define JOY_UP_PIN      GPIO_PIN_14

#define JOY_DOWN_PORT   GPIOF
#define JOY_DOWN_PIN    GPIO_PIN_0

#define JOY_LEFT_PORT   GPIOC
#define JOY_LEFT_PIN    GPIO_PIN_13

#define JOY_RIGHT_PORT  GPIOC
#define JOY_RIGHT_PIN   GPIO_PIN_15

#define JOY_CENTER_PORT GPIOF
#define JOY_CENTER_PIN  GPIO_PIN_1

typedef enum {
    STATE_START_SCREEN,
    STATE_MAIN_MENU,
    STATE_NAME_INPUT,
    STATE_RACING,
    STATE_RESULT
} GameState;

GameState currentState = STATE_START_SCREEN;
uint8_t startScreenDrawn = 0;
uint8_t menuSelection = 0;

void SysTick_Handler(void) {
    HAL_IncTick();
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);
}

void GPIO_TFT_Init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = TFT_CS_PIN | TFT_RST_PIN | TFT_DC_PIN | TFT_BL_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio);

    gpio.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF0_SPI1;
    HAL_GPIO_Init(GPIOA, &gpio);
}

void SPI1_Init(void) {
    __HAL_RCC_SPI1_CLK_ENABLE();

    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    HAL_SPI_Init(&hspi1);
}

void GPIO_Joystick_Init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = JOY_UP_PIN | JOY_LEFT_PIN | JOY_DOWN_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = JOY_RIGHT_PIN | JOY_CENTER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

void UART5_Init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART5_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART5;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    huart5.Instance = USART5;
    huart5.Init.BaudRate = 9600;
    huart5.Init.WordLength = UART_WORDLENGTH_8B;
    huart5.Init.StopBits = UART_STOPBITS_1;
    huart5.Init.Parity = UART_PARITY_NONE;
    huart5.Init.Mode = UART_MODE_TX_RX;
    huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart5.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart5);
}

void sendString(const char *s) {
    HAL_UART_Transmit(&huart5, (uint8_t *)s, strlen(s), 100);
}

void SendLedCmd(char prefix, int ledNum) {
    char cmd[4] = {prefix, (char)('0' + ledNum), '\r', '\n'};
    HAL_UART_Transmit(&huart5, (uint8_t *)cmd, 4, 100);
}

void CollectReactionTimes(uint32_t timeout_ms) {
    uint32_t startTime = HAL_GetTick();
    char rxBuf[8] = {0};
    int idx = 0;
    playerTimes[0] = playerTimes[1] = 0;
    playerFinished[0] = playerFinished[1] = 0;
    uint8_t stillWaiting = 2;

    while (HAL_GetTick() - startTime < timeout_ms && stillWaiting > 0) {
        uint8_t rxByte;
        if (HAL_UART_Receive(&huart5, &rxByte, 1, 10) == HAL_OK) {
            if (rxByte == '\n' || rxByte == '\r') {
                if (idx >= 2 && rxBuf[0] == 'B') {
                    int p = rxBuf[1] - '0';
                    if (p >= 1 && p <= 2 && !playerFinished[p - 1]) {
                        playerTimes[p - 1] = HAL_GetTick() - startTime;
                        playerFinished[p - 1] = 1;
                        stillWaiting--;
                    }
                }
                idx = 0;
                memset(rxBuf, 0, sizeof(rxBuf));
            } else if (idx < 7) {
                rxBuf[idx++] = rxByte;
            }
        }
    }
}

void UpdateNameDisplay(uint8_t cursor) {
    TFT_DrawString(100, 85, p1_name, COLOR_WHITE, COLOR_BLACK, 3);
    TFT_DrawString(100, 145, p2_name, COLOR_WHITE, COLOR_BLACK, 3);

    uint16_t y_line_p1 = 85 + 24;
    uint16_t y_line_p2 = 145 + 24;

    TFT_FillRect(100, y_line_p1, 15, 4, COLOR_BLACK);
    TFT_FillRect(118, y_line_p1, 15, 4, COLOR_BLACK);
    TFT_FillRect(136, y_line_p1, 15, 4, COLOR_BLACK);

    TFT_FillRect(100, y_line_p2, 15, 4, COLOR_BLACK);
    TFT_FillRect(118, y_line_p2, 15, 4, COLOR_BLACK);
    TFT_FillRect(136, y_line_p2, 15, 4, COLOR_BLACK);

    if (cursor < 3) {
        TFT_FillRect(100 + (cursor * 18), y_line_p1, 15, 4, COLOR_YELLOW);
    } else if (cursor < 6) {
        TFT_FillRect(100 + ((cursor - 3) * 18), y_line_p2, 15, 4, COLOR_YELLOW);
    }
}

void DrawCheckeredFlag(uint16_t start_y, uint16_t height) {
    uint8_t size = 16;
    for (int y = 0; y < height; y += size) {
        for (int x = 0; x < 320; x += size) {
            uint16_t color = ((x / size) + (y / size)) % 2 == 0 ? COLOR_WHITE : COLOR_BLACK;
            TFT_FillRect(x, start_y + y, size, size, color);
        }
    }
}

void DrawF1Logo(uint16_t x, uint16_t y) {
    TFT_FillRect(x, y, 30, 8, COLOR_RED);
    TFT_FillRect(x, y, 8, 40, COLOR_RED);
    TFT_FillRect(x, y + 16, 20, 8, COLOR_RED);

    TFT_FillRect(x + 35, y + 10, 25, 30, COLOR_WHITE);
    TFT_FillRect(x + 25, y + 15, 10, 8, COLOR_WHITE);

    TFT_FillRect(x + 30, y + 25, 30, 4, COLOR_BLACK);
    TFT_FillRect(x + 28, y + 32, 30, 4, COLOR_BLACK);
}

void DrawMainMenu(uint8_t selection) {
    TFT_FillRect(0, 0, 320, 240, COLOR_BLACK);

    TFT_FillRect(0, 0, 320, 35, COLOR_RED);
    TFT_DrawString(80, 10, "MAIN MENU", COLOR_WHITE, COLOR_RED, 2);
    DrawCheckeredFlag(208, 32);

    if (selection == 0) {
        TFT_DrawString(40, 80, "> NEW GAME", COLOR_GREEN, COLOR_BLACK, 2);
        TFT_DrawString(40, 120, "  LEADERBOARD", COLOR_WHITE, COLOR_BLACK, 2);
    } else {
        TFT_DrawString(40, 80, "  NEW GAME", COLOR_WHITE, COLOR_BLACK, 2);
        TFT_DrawString(40, 120, "> LEADERBOARD", COLOR_GREEN, COLOR_BLACK, 2);
    }

    TFT_DrawString(10, 180, "L: START SCREEN | C: SELECT", COLOR_YELLOW, COLOR_BLACK, 1);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    GPIO_TFT_Init();
    GPIO_Joystick_Init();
    SPI1_Init();
    UART5_Init();

    HAL_GPIO_WritePin(TFT_BL_PORT, TFT_BL_PIN, GPIO_PIN_RESET);
    TFT_Reset();
    TFT_Init();

    while (1) {
        switch (currentState) {

            case STATE_START_SCREEN:
                if (HAL_GPIO_ReadPin(JOY_CENTER_PORT, JOY_CENTER_PIN) == GPIO_PIN_RESET) {
                    currentState = STATE_MAIN_MENU;
                    menuSelection = 0;

                    TFT_FillRect(0, 0, 320, 240, COLOR_BLACK);

                    TFT_FillRect(0, 0, 320, 35, COLOR_RED);
                    TFT_DrawString(80, 10, "MAIN MENU", COLOR_WHITE, COLOR_RED, 2);
                    DrawCheckeredFlag(208, 32);

                    TFT_DrawString(40, 80, "> NEW GAME", COLOR_GREEN, COLOR_BLACK, 2);
                    TFT_DrawString(40, 120, "  LEADERBOARD", COLOR_WHITE, COLOR_BLACK, 2);

                    TFT_DrawString(10, 180, "L: START SCREEN | C: SELECT", COLOR_YELLOW, COLOR_BLACK, 1);

                    while(HAL_GPIO_ReadPin(JOY_CENTER_PORT, JOY_CENTER_PIN) == GPIO_PIN_RESET);
                }

                if (!startScreenDrawn) {
                    TFT_FillRect(0, 0, 320, 240, COLOR_BLACK);

                    DrawCheckeredFlag(0, 32);
                    DrawCheckeredFlag(240 - 32, 32);

                    DrawF1Logo(120, 55);

                    TFT_DrawString(40, 120, "REACTION RACE", COLOR_RED, COLOR_BLACK, 3);
                    TFT_DrawString(60, 180, "PRESS CENTER TO START", COLOR_YELLOW, COLOR_BLACK, 2);

                    startScreenDrawn = 1;
                }
                break;

            case STATE_MAIN_MENU:
                if (HAL_GPIO_ReadPin(JOY_LEFT_PORT, JOY_LEFT_PIN) == GPIO_PIN_RESET) {
                    currentState = STATE_START_SCREEN;
                    startScreenDrawn = 0;
                    while(HAL_GPIO_ReadPin(JOY_LEFT_PORT, JOY_LEFT_PIN) == GPIO_PIN_RESET);
                }

                if (HAL_GPIO_ReadPin(JOY_DOWN_PORT, JOY_DOWN_PIN) == GPIO_PIN_RESET ||
                    HAL_GPIO_ReadPin(JOY_UP_PORT, JOY_UP_PIN) == GPIO_PIN_RESET) {

                    menuSelection = (menuSelection == 0) ? 1 : 0;

                    if (menuSelection == 0) {
                        TFT_DrawString(40, 80, "> NEW GAME", COLOR_GREEN, COLOR_BLACK, 2);
                        TFT_DrawString(40, 120, "  LEADERBOARD", COLOR_WHITE, COLOR_BLACK, 2);
                    } else {
                        TFT_DrawString(40, 80, "  NEW GAME", COLOR_WHITE, COLOR_BLACK, 2);
                        TFT_DrawString(40, 120, "> LEADERBOARD", COLOR_GREEN, COLOR_BLACK, 2);
                    }
                    while(HAL_GPIO_ReadPin(JOY_DOWN_PORT, JOY_DOWN_PIN) == GPIO_PIN_RESET ||
                          HAL_GPIO_ReadPin(JOY_UP_PORT, JOY_UP_PIN) == GPIO_PIN_RESET);
                }
                if (HAL_GPIO_ReadPin(JOY_CENTER_PORT, JOY_CENTER_PIN) == GPIO_PIN_RESET) {
                    if (menuSelection == 0) {
                        currentState = STATE_NAME_INPUT;
                        nameCursor = 0;

                        TFT_FillRect(0, 0, 320, 240, COLOR_BLACK);
                        TFT_FillRect(0, 0, 320, 35, COLOR_RED);
                        TFT_DrawString(15, 10, "DRIVER REGISTRATION", COLOR_WHITE, COLOR_RED, 2);

                        TFT_DrawString(40, 85, "P1:", COLOR_WHITE, COLOR_BLACK, 2);
                        TFT_DrawString(40, 145, "P2:", COLOR_WHITE, COLOR_BLACK, 2);

                        TFT_DrawString(5, 205, "LEFT ON P1: BACK TO MENU", COLOR_RED, COLOR_BLACK, 1);
                        TFT_DrawString(5, 220, "UP/DN: EDIT | R/L: MOVE | OK: START", COLOR_YELLOW, COLOR_BLACK, 1);

                        UpdateNameDisplay(nameCursor);

                    } else if (menuSelection == 1) {
                        currentState = STATE_RESULT;
                        TFT_FillRect(0, 0, 320, 240, COLOR_BLACK);
                        TFT_FillRect(0, 0, 320, 35, COLOR_RED);
                        TFT_DrawString(80, 10, "LEADERBOARD", COLOR_WHITE, COLOR_RED, 2);

                        if (currentLeaderboardEntries == 0) {
                            TFT_DrawString(40, 100, "NO TIMES YET", COLOR_YELLOW, COLOR_BLACK, 2);
                        } else {
                            for(int i = 0; i < currentLeaderboardEntries; i++) {
                                char rankStr[3] = {'1' + i, '.', '\0'};
                                TFT_DrawString(20, 60 + (i * 30), rankStr, COLOR_WHITE, COLOR_BLACK, 2);
                                TFT_DrawString(60, 60 + (i * 30), leaderboard[i].name, COLOR_GREEN, COLOR_BLACK, 2);

                                char timeStr[10];
                                IntToStr(leaderboard[i].time_ms, timeStr);
                                TFT_DrawString(140, 60 + (i * 30), timeStr, COLOR_WHITE, COLOR_BLACK, 2);
                                TFT_DrawString(240, 60 + (i * 30), "ms", COLOR_YELLOW, COLOR_BLACK, 2);
                            }
                        }
                        TFT_DrawString(10, 210, "CENTER BUTTON: BACK TO MENU", COLOR_YELLOW, COLOR_BLACK, 1);
                    }
                    while(HAL_GPIO_ReadPin(JOY_CENTER_PORT, JOY_CENTER_PIN) == GPIO_PIN_RESET);
                }
                break;

            case STATE_NAME_INPUT:
                if (HAL_GPIO_ReadPin(JOY_UP_PORT, JOY_UP_PIN) == GPIO_PIN_RESET) {
                    if (nameCursor < 3) {
                        p1_name[nameCursor]++;
                        if (p1_name[nameCursor] > 'Z') p1_name[nameCursor] = 'A';
                    } else if (nameCursor < 6) {
                        p2_name[nameCursor - 3]++;
                        if (p2_name[nameCursor - 3] > 'Z') p2_name[nameCursor - 3] = 'A';
                    }
                    UpdateNameDisplay(nameCursor);
                    while(HAL_GPIO_ReadPin(JOY_UP_PORT, JOY_UP_PIN) == GPIO_PIN_RESET);
                }
                if (HAL_GPIO_ReadPin(JOY_DOWN_PORT, JOY_DOWN_PIN) == GPIO_PIN_RESET) {
                    if (nameCursor < 3) {
                        p1_name[nameCursor]--;
                        if (p1_name[nameCursor] < 'A') p1_name[nameCursor] = 'Z';
                    } else if (nameCursor < 6) {
                        p2_name[nameCursor - 3]--;
                        if (p2_name[nameCursor - 3] < 'A') p2_name[nameCursor - 3] = 'Z';
                    }
                    UpdateNameDisplay(nameCursor);
                    while(HAL_GPIO_ReadPin(JOY_DOWN_PORT, JOY_DOWN_PIN) == GPIO_PIN_RESET);
                }

                if (HAL_GPIO_ReadPin(JOY_RIGHT_PORT, JOY_RIGHT_PIN) == GPIO_PIN_RESET) {
                    if (nameCursor < 5) nameCursor++;
                    UpdateNameDisplay(nameCursor);
                    while(HAL_GPIO_ReadPin(JOY_RIGHT_PORT, JOY_RIGHT_PIN) == GPIO_PIN_RESET);
                }

                if (HAL_GPIO_ReadPin(JOY_LEFT_PORT, JOY_LEFT_PIN) == GPIO_PIN_RESET) {
                    if (nameCursor > 0) {
                        nameCursor--;
                        UpdateNameDisplay(nameCursor);
                    } else {
                        currentState = STATE_MAIN_MENU;
                        menuSelection = 0;
                        DrawMainMenu(menuSelection);
                    }
                    while(HAL_GPIO_ReadPin(JOY_LEFT_PORT, JOY_LEFT_PIN) == GPIO_PIN_RESET);
                }

                if (HAL_GPIO_ReadPin(JOY_CENTER_PORT, JOY_CENTER_PIN) == GPIO_PIN_RESET) {
                    currentState = STATE_RACING;
                    while(HAL_GPIO_ReadPin(JOY_CENTER_PORT, JOY_CENTER_PIN) == GPIO_PIN_RESET);
                }

                break;

            case STATE_RACING: {
                TFT_FillRect(0, 0, 320, 240, COLOR_BLACK);
                TFT_DrawString(110, 30, "READY", COLOR_YELLOW, COLOR_BLACK, 3);

                for(int i = 0; i < 5; i++) {
                    TFT_FillRect(30 + (i * 55), 100, 40, 40, COLOR_WHITE);
                    TFT_FillRect(32 + (i * 55), 102, 36, 36, COLOR_BLACK);
                }

                HAL_Delay(1000);

                for(int i = 0; i < 5; i++) {
                    TFT_FillRect(32 + (i * 55), 102, 36, 36, COLOR_RED);
                    SendLedCmd('L', i + 1);
                    HAL_Delay(1000);
                }

                uint32_t random_delay = 500 + (HAL_GetTick() % 2000);
                HAL_Delay(random_delay);

                for(int i = 0; i < 5; i++) {
                    TFT_FillRect(32 + (i * 55), 102, 36, 36, COLOR_BLACK);
                    SendLedCmd('l', i + 1);
                }

                TFT_DrawString(110, 30, " GO! ", COLOR_GREEN, COLOR_BLACK, 3);

                CollectReactionTimes(5000);

                TFT_FillRect(0, 0, 320, 240, COLOR_BLACK);
                TFT_DrawString(80, 20, "RESULTS", COLOR_WHITE, COLOR_RED, 2);

                char timeStr[10];
                if (playerFinished[0]) {
                    IntToStr(playerTimes[0], timeStr);
                    TFT_DrawString(20, 80, p1_name, COLOR_GREEN, COLOR_BLACK, 2);
                    TFT_DrawString(120, 80, timeStr, COLOR_WHITE, COLOR_BLACK, 2);
                    TFT_DrawString(220, 80, "ms", COLOR_YELLOW, COLOR_BLACK, 2);
                    AddLeaderboardEntry(p1_name, playerTimes[0]);
                } else {
                    TFT_DrawString(20, 80, p1_name, COLOR_GREEN, COLOR_BLACK, 2);
                    TFT_DrawString(120, 80, "DNF", COLOR_RED, COLOR_BLACK, 2);
                }

                if (playerFinished[1]) {
                    IntToStr(playerTimes[1], timeStr);
                    TFT_DrawString(20, 120, p2_name, COLOR_GREEN, COLOR_BLACK, 2);
                    TFT_DrawString(120, 120, timeStr, COLOR_WHITE, COLOR_BLACK, 2);
                    TFT_DrawString(220, 120, "ms", COLOR_YELLOW, COLOR_BLACK, 2);
                    AddLeaderboardEntry(p2_name, playerTimes[1]);
                } else {
                    TFT_DrawString(20, 120, p2_name, COLOR_GREEN, COLOR_BLACK, 2);
                    TFT_DrawString(120, 120, "DNF", COLOR_RED, COLOR_BLACK, 2);
                }

                TFT_DrawString(10, 200, "CENTER: BACK TO MENU", COLOR_YELLOW, COLOR_BLACK, 1);

                while(HAL_GPIO_ReadPin(JOY_CENTER_PORT, JOY_CENTER_PIN) != GPIO_PIN_RESET);

                currentState = STATE_MAIN_MENU;
                menuSelection = 0;
                DrawMainMenu(menuSelection);
                while(HAL_GPIO_ReadPin(JOY_CENTER_PORT, JOY_CENTER_PIN) == GPIO_PIN_RESET);
                break;
            }

            case STATE_RESULT:
                if (HAL_GPIO_ReadPin(JOY_CENTER_PORT, JOY_CENTER_PIN) == GPIO_PIN_RESET) {
                    currentState = STATE_MAIN_MENU;
                    menuSelection = 0;
                    DrawMainMenu(menuSelection);
                    while(HAL_GPIO_ReadPin(JOY_CENTER_PORT, JOY_CENTER_PIN) == GPIO_PIN_RESET);
                }
                break;
        }
    }
}
