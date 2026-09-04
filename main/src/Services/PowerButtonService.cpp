#include "PowerButtonService.h"
#include <esp_log.h>
#include "Devices/Input.h"
#include "Services/Audio.h"
#include "Util/Services.h"
#include "Util/stdafx.h"

static const char* TAG = "PowerButtonService";

extern void gracefulShutdown(const char* audioFile);

PowerButtonService::PowerButtonService() : Threaded("PowerButton", 3 * 1024), queue(6){
	Events::listen(Facility::Input, &queue);
	start();
}

PowerButtonService::~PowerButtonService(){
	stop(0);
	queue.unblock();

	while(running()){
		delayMillis(1);
	}

	Events::unlisten(&queue);
}

void PowerButtonService::loop(){
	Event event{};
	if(!queue.get(event, portMAX_DELAY)) return;

	bool powerOff = false;

	if(event.facility == Facility::Input && event.data != nullptr){
		const auto* data = (Input::Data*) event.data;
		powerOff = data->btn == Input::Power && data->action == Input::Data::Hold;
	}

	free(event.data);

	if(!powerOff) return;

	ESP_LOGI(TAG, "Power button held, shutting down");

	Events::unlisten(&queue);

	if(Audio* audio = (Audio*) Services.get(Service::Audio)){
		audio->play("/spiffs/Beep3.aac", true);
		delayMillis(400);
	}

	gracefulShutdown(nullptr);
}
