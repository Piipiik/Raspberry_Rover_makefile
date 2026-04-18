/* App/Sensor/app_sensor.c */
#include "app_sensor.h"
#include "app.h"
#include "mpu6050.h"

uint8_t g_mpu_id = 0;
SoftI2C_Type MPU6050_I2C;

void SensorAcquisitionTask(void *argument)
{
    // 粘贴原来的代码
    MPU6050_I2C.port = GPIOA;
    MPU6050_I2C.scl_pin = GPIO_PIN_9;
    MPU6050_I2C.sda_pin = GPIO_PIN_10;
    MPU6050_Init(&MPU6050_I2C);
    g_mpu_id = MPU6050_GetID();

    for(;;) {
        osDelay(10);
    }
}