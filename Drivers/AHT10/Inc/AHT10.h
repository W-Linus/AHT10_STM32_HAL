#ifndef __AHT10_H_
#define __AHT10_H_

#include <stm32g4xx_hal.h>
#include <i2c.h>

#define AHT10_ADDRESS   0x70  //器件地址是0x38，但是需要左移一位，所以实际上地址是0x70
#define AHT10_I2C       hi2c3

#define Initialize_Command      0xE1
#define Measure_Trig_Command    0xAC
#define Soft_Reset_Command      0xBA


void AHT10_Init(void);
void AHT10_Reset(void);

uint8_t AHT10_Measure(float *pTemperature, float *pHumidity);
void AHT10_Trig_Measure(void);
uint8_t AHT10_Read_Data(uint8_t *pOriginData);

float AHT10_GetHumidity(void);
float AHT10_GetTemperature(void);

#endif
