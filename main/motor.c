#include <esp_log.h>
#include <esp_err.h>

#include <driver/mcpwm.h>

#include "car.h"

void motor_init(void) {
    mcpwm_pin_config_t mcpwm_pin_config = {
        .mcpwm0a_out_num = 13,
        .mcpwm0b_out_num = 14,
        .mcpwm1a_out_num = 15,
        .mcpwm1b_out_num = 12,
    };
    ESP_ERROR_CHECK(mcpwm_set_pin(MCPWM_UNIT_0, &mcpwm_pin_config));

    mcpwm_config_t mcpwm_config = {
        .frequency   = 1000,
        .duty_mode = MCPWM_DUTY_MODE_0,
        .counter_mode = MCPWM_UP_COUNTER,
    };
    ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &mcpwm_config));
    ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &mcpwm_config));
}

void motor_move(float left, float right) {
    ESP_LOGI("motor", "%f, %f", left, right);
    if(left > 0) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, left);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
    } else if(left < 0) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, -left);
    } else {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0);
    }

    if(right > 0) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, right);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, 0);
    } else if(right < 0) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, -right);
    } else {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, 0);
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, 0);
    }
}
