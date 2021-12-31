#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/mcpwm.h>
#include <esp_err.h>

void app_main(void)
{
    gpio_pad_select_gpio(4);
    gpio_pad_select_gpio(2);
    gpio_set_direction(4, GPIO_MODE_OUTPUT);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);

    mcpwm_pin_config_t mcpwm_pin_config = {
        .mcpwm0a_out_num = 13,
        .mcpwm0b_out_num = 14,
        .mcpwm1a_out_num = 15,
        .mcpwm1b_out_num = 12,
    };
    mcpwm_set_pin(MCPWM_UNIT_0, &mcpwm_pin_config);

    mcpwm_config_t mcpwm_config = {
        .frequency   = 1000,
        .counter_mode = MCPWM_UP_COUNTER,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &mcpwm_config);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &mcpwm_config);

    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);

    while(1) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 50);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 50);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        //gpio_set_level(4, 1);
        //vTaskDelay(1000 / portTICK_PERIOD_MS);

        //gpio_set_level(4, 0);
        //vTaskDelay(1000 / portTICK_PERIOD_MS);

        gpio_set_level(2, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        gpio_set_level(2, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
