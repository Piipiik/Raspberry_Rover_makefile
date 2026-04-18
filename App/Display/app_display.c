/* App/Sensor/app_sensor.c */
#include "app_display.h"
#include "app.h"
#include "oled.h"

SoftI2C_Type OLED_I2C;

void UserInterfaceTask(void *argument)
{
  OLED_I2C.port = GPIOB;
  OLED_I2C.scl_pin = GPIO_PIN_10;
  OLED_I2C.sda_pin = GPIO_PIN_11;
  OLED_Init(&OLED_I2C);
  int16_t count = 0;

    for(;;) {
            // count++;
            // // OLED_ShowNumF6X16(0, 0, encoder_ticks[0], 4, 1);
            // // OLED_ShowNumF6X16(0, 2, (int16_t)(target_vx * 10), 4, 1);
            // // OLED_ShowNumF6X16(0, 4, (int16_t)(target_vy * 10), 4, 1);
            // // OLED_ShowNumF6X16(0, 6, (int16_t)(target_wz * 10), 4, 1);
            // // OLED_ShowNumF6X16(1, 6, g_mpu_id, 4, 1);
            // // OLED_ShowFloatF6X16(0, 0, target_vx, 5, 2, 1);
            // // OLED_ShowFloatF6X16(0, 2, target_wheel_speed[0], 5, 2, 1);
            // OLED_ShowFloatF6X16(0, 0, current_vx, 5, 3, 1);
            // OLED_ShowFloatF6X16(0, 2, current_vy, 5, 3, 1);
            // OLED_ShowFloatF6X16(0, 4, current_wz, 5, 3, 1);
            // OLED_ShowNumF6X16(100, 0, count, 4, 1);
            // // OLED_ShowNumF6X16(100, 2, GetTaskStackWaterMark, 4, 1);
            osDelay(200);
    }
}