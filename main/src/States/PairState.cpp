#include "PairState.h"
#include "Util/Services.h"
#include "Services/LEDService.h"
#include "Devices/Input.h"
#include "Services/Audio.h"
#include "States/DriveState/DriveState.h"

PairState::PairState() : State(), queue(10), audio((Audio*) Services.get(Service::Audio)){
	Events::listen(Facility::Input, &queue);
	Events::listen(Facility::Pair, &queue);

	if (LEDService* led = (LEDService*)Services.get(Service::LED)) {
		led->on(LED::StatusYellow);
	}

	if(auto input = (Input*) Services.get(Service::Input)){
		if(input->getState(Input::Pair)){
			startPair();
		}
	}

	if(WiFiAP* wifi = (WiFiAP*) Services.get(Service::WiFi)){
		wifi->setHidden(true);
		wifi->generateNewSSID();
	}
	auto* audio = (Audio*) Services.get(Service::Audio);
	audio->setEnabled(true);
}

PairState::~PairState() {
	if (LEDService* led = (LEDService*)Services.get(Service::LED)) {
		led->off(LED::StatusYellow);
	}

	Events::unlisten(&queue);
}

void PairState::loop() {
	LEDService* led = (LEDService*)Services.get(Service::LED);
	if (led == nullptr) {
		return;
	}

	Event event{};
	if (!queue.get(event, portMAX_DELAY)) {
		return;
	}

	if (event.facility == Facility::Input) {
		const Input::Data* data = (Input::Data*)event.data;
		if(data != nullptr && data->action == Input::Data::Press){
			startPair();
		}else{
			stopPair();
		}
	}
	else if (event.facility == Facility::Pair) {
		const PairService::Event* pairEvent = (PairService::Event*)event.data;
		if (pairEvent != nullptr && pairEvent->success) {
			audio->play("/spiffs/General/PairSuccess.aac", true);

			if (StateMachine* stateMachine = (StateMachine*)Services.get(Service::StateMachine)) {
				stateMachine->transition<DriveState>();
			}
		}else if(pairEvent != nullptr && !pairEvent->success){
			audio->play("/spiffs/General/PairFail.aac", true);
		}
	}

	free(event.data);
}

void PairState::startPair(){
	if(pairService) return;

	pairService = std::make_unique<PairService>();

	if(LEDService* led = (LEDService*) Services.get(Service::LED)){
		led->off(LED::StatusRed);
		led->blink(LED::StatusYellow, 0);
	}

	if(audio->getCurrentPlayingFile() != "/spiffs/General/PairStart.aac"){
		audio->play("/spiffs/General/PairStart.aac", true);
	}
}

void PairState::stopPair(){
	if(!pairService) return;

	pairService.reset();

	if(WiFiAP* wifi = (WiFiAP*) Services.get(Service::WiFi)){
		wifi->setHidden(true);
		wifi->generateNewSSID();
	}

	if(LEDService* led = (LEDService*) Services.get(Service::LED)){
		led->on(LED::StatusYellow);
	}
}

void PairState::unblock(){
	queue.unblock();
}
