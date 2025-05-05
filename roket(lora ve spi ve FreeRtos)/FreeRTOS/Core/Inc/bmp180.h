#ifndef _BMP180_H_
#define _BMP180_H_

#include "stm32f4xx_hal.h"


typedef struct {
    I2C_HandleTypeDef *hi2c;
    int16_t AC1, AC2, AC3, B1, B2, MB, MC, MD;
    uint16_t AC4, AC5, AC6;
    int32_t B5;
} BMP180_t;

uint8_t BMP180_Init(BMP180_t *bmp, I2C_HandleTypeDef *hi2c);
float BMP180_ReadTemperature(BMP180_t *bmp);
int32_t BMP180_ReadPressure(BMP180_t *bmp);

#endif
