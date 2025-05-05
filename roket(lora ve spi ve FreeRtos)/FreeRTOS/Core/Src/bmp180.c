#include "bmp180.h"
#include "math.h"

#define BMP180_ADDR 0xEE

static uint16_t read16(BMP180_t *bmp, uint8_t reg) {
    uint8_t buf[2];
    HAL_I2C_Mem_Read(bmp->hi2c, BMP180_ADDR, reg, 1, buf, 2, HAL_MAX_DELAY);
    return (buf[0] << 8) | buf[1];
}

static int16_t readS16(BMP180_t *bmp, uint8_t reg) {
    return (int16_t)read16(bmp, reg);
}

uint8_t BMP180_Init(BMP180_t *bmp, I2C_HandleTypeDef *hi2c) {
    bmp->hi2c = hi2c;

    bmp->AC1 = readS16(bmp, 0xAA);
    bmp->AC2 = readS16(bmp, 0xAC);
    bmp->AC3 = readS16(bmp, 0xAE);
    bmp->AC4 = read16(bmp, 0xB0);
    bmp->AC5 = read16(bmp, 0xB2);
    bmp->AC6 = read16(bmp, 0xB4);
    bmp->B1  = readS16(bmp, 0xB6);
    bmp->B2  = readS16(bmp, 0xB8);
    bmp->MB  = readS16(bmp, 0xBA);
    bmp->MC  = readS16(bmp, 0xBC);
    bmp->MD  = readS16(bmp, 0xBE);

    return 1;
}

float BMP180_ReadTemperature(BMP180_t *bmp) {
    uint8_t cmd = 0x2E;
    HAL_I2C_Mem_Write(bmp->hi2c, BMP180_ADDR, 0xF4, 1, &cmd, 1, HAL_MAX_DELAY);
    HAL_Delay(5);

    int32_t UT = read16(bmp, 0xF6);

    int32_t X1 = ((UT - (int32_t)bmp->AC6) * (int32_t)bmp->AC5) >> 15;
    int32_t X2 = ((int32_t)bmp->MC << 11) / (X1 + bmp->MD);
    bmp->B5 = X1 + X2;
    return ((bmp->B5 + 8) >> 4) / 10.0;
}

int32_t BMP180_ReadPressure(BMP180_t *bmp) {
    uint8_t cmd = 0x34;
    HAL_I2C_Mem_Write(bmp->hi2c, BMP180_ADDR, 0xF4, 1, &cmd, 1, HAL_MAX_DELAY);
    HAL_Delay(5);

    int32_t UP = read16(bmp, 0xF6);

    int32_t B6 = bmp->B5 - 4000;
    int32_t X1 = (bmp->B2 * ((B6 * B6) >> 12)) >> 11;
    int32_t X2 = (bmp->AC2 * B6) >> 11;
    int32_t X3 = X1 + X2;
    int32_t B3 = (((((int32_t)bmp->AC1) * 4 + X3) + 2) / 4);
    X1 = (bmp->AC3 * B6) >> 13;
    X2 = (bmp->B1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    uint32_t B4 = (bmp->AC4 * (uint32_t)(X3 + 32768)) >> 15;
    uint32_t B7 = ((uint32_t)(UP - B3)) * 50000;
    int32_t p = B7 / B4 * 2;

    X1 = (p >> 8) * (p >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * p) >> 16;
    return p + ((X1 + X2 + 3791) >> 4);
}
