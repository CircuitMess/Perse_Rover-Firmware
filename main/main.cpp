#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "Util/Services.h"
#include "Util/stdafx.h"
#include "Util/HWVersion.h"
#include "Pins.hpp"
#include "Periph/I2C.h"
#include "Periph/SPIFFS.h"
#include "Devices/AW9523.h"
#include "Devices/Input.h"
#include "Devices/Power.h"
#include "Services/Audio.h"
#include "Services/StateMachine.h"
#include "Services/PowerButtonService.h"
#include "States/AudioTestState.h"

/**
 * Curiosity audio sample test build. The device is a bare main PCB with a speaker and the power button:
 * no camera, motors, modules or WiFi. It cycles through the voice candidates in AudioTestState, one per
 * button press. See TASK.md on this branch.
 */

[[noreturn]] void shutdown(){
	Power::off();
}

/**
 * Stops playback, mutes the amp and releases the power latch. Kept under the same name as in the full
 * firmware because PowerButtonService and InactivityService link against it.
 */
[[noreturn]] void gracefulShutdown(const char* audioFile){
	if(StateMachine* stateMachine = (StateMachine*) Services.get(Service::StateMachine)){
		Services.set(Service::StateMachine, nullptr);
		delete stateMachine;
	}

	if(Audio* audio = (Audio*) Services.get(Service::Audio)){
		Services.set(Service::Audio, nullptr);
		if(audioFile != nullptr){
			audio->play(audioFile, true);
			delayMillis(3000);
		}
		delete audio;
	}

	shutdown();
}

void init(){
	esp_log_level_set("*", ESP_LOG_WARN);
	esp_log_level_set("AudioTestState", ESP_LOG_INFO);

	/* The board only stays powered while the latch is driven, so this has to happen before anything
	 * that can take time. The bootloader hook already asserted it - this claims the pin properly. */
	Power::hold();

	if(!HWVersion::check()){
		while(true){
			vTaskDelay(1000);
			HWVersion::log();
		}
	}

	auto i2c = new I2C(I2C_NUM_0, (gpio_num_t) I2C_SDA, (gpio_num_t) I2C_SCL);
	auto aw9523 = new AW9523(*i2c, 0x5b);

	new SPIFFS();

	auto audio = new Audio(*aw9523);
	Services.set(Service::Audio, audio);

	auto input = new Input();
	Services.set(Service::Input, input);

	auto stateMachine = new StateMachine();
	Services.set(Service::StateMachine, stateMachine);

	// Hold -> Beep3 -> gracefulShutdown -> power off.
	new PowerButtonService();

	stateMachine->transition<AudioTestState>();
	stateMachine->begin();
}

extern "C" void app_main(void){
	init();
	vTaskDelete(nullptr);
}
