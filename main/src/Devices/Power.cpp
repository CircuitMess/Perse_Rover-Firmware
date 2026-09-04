#include "Power.h"
#include <driver/gpio.h>
#include <esp_sleep.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Pins.hpp"

static const char* TAG = "Power";

void Power::hold(){
	const gpio_config_t cfg = {
			.pin_bit_mask = 1ULL << PIN_PWR_HOLD,
			.mode = GPIO_MODE_OUTPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&cfg);
	gpio_set_level((gpio_num_t) PIN_PWR_HOLD, 1);
}

void Power::off(){
	ESP_LOGI(TAG, "Releasing power latch");

	gpio_set_level((gpio_num_t) PIN_PWR_HOLD, 0);

	/* The latch takes a moment to drop, and while the rover is plugged into USB the 5V rail keeps
	 * the charger alive even though V_SYS is gone. Fall back to deep sleep so we don't spin here. */
	vTaskDelay(pdMS_TO_TICKS(200));

	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RC_FAST, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_CPU, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_AUTO));
	ESP_ERROR_CHECK(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL));
	esp_deep_sleep_start();
}
