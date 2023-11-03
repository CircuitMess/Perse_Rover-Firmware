#include "DemoState.h"
#include "Devices/Input.h"
#include "Services/StateMachine.h"
#include "Util/Services.h"
#include "PairState.h"
#include "Util/stdafx.h"
#include "Services/LEDService.h"
#include "Pins.hpp"

const DemoState::Action DemoState::Actions[] = {
		{ .type = Action::Cam, .pos = 0, .duration = 1000 },
		{ .type = Action::Cam, .pos = 100, .duration = 1000 },
		{ .type = Action::Cam, .pos = 50, .duration = 500 },

		{ .type = Action::Arm, .pos = 40, .duration = 500 },
		{ .type = Action::Pinch, .pos = 0, .duration = 1000 },
		{ .type = Action::Arm, .pos = 100, .duration = 500 },

		{ .type = Action::Move, .dir = 2, .duration = 4000 },

		{ .type = Action::Arm, .pos = 50, .duration = 500 },
		{ .type = Action::Pinch, .pos = 100, .duration = 1000 },
		{ .type = Action::Arm, .pos = 100, .duration = 500 },

		{ .type = Action::Cam, .pos = 0, .duration = 1000 },
		{ .type = Action::Cam, .pos = 100, .duration = 1000 },
		{ .type = Action::Cam, .pos = 50, .duration = 500 },

		{ .type = Action::Move, .dir = 6, .duration = 4000 },

		{ .type = Action::Pause, .duration = 1000 },

};

DemoState::DemoState(MotorDriveController& motors, ArmController& arm, CameraController& cam) : evts(12), motors(motors), arm(arm), cam(cam){

	Events::listen(Facility::Input, &evts);

	if(LEDService* led = (LEDService*) Services.get(Service::LED)){
		led->blink(LED::StatusGreen, 0);

		led->on(LED::Rear);

		led->blink(LED::RightMotor, 0);
		led->blink(LED::LeftMotor, 0);

		led->on(LED::LeftHeadlight);
		led->on(LED::RightHeadlight);

		led->on(LED::StatusRed);
	}

	cam.setControl(Local);
	arm.setControl(Local);
	motors.setControl(Local);

	doAction(Actions[0]);
	actionTime = millis();
}

DemoState::~DemoState(){
	Events::unlisten(&evts);

	stopAction(Actions[actionIndex]);

	cam.setControl(Remote);
	arm.setControl(Remote);
	motors.setControl(Remote);

	if(LEDService* ledService = (LEDService*) Services.get(Service::LED)){
		for(const LED led : { LED::StatusRed, LED::StatusGreen, LED::Rear, LED::LeftMotor, LED::RightMotor, LED::LeftHeadlight, LED::RightHeadlight }){
			ledService->off(led);
		}
	}
}

void DemoState::loop(){
	Event evt{};
	if(evts.get(evt, 5)){
		auto data = (Input::Data*) evt.data;
		if(data->btn == Input::Pair && data->action == Input::Data::Press){
			free(evt.data);

			auto stateMachine = (StateMachine*) Services.get(Service::StateMachine);
			stateMachine->transition<PairState>();

			return;
		}
		free(evt.data);
	}

	if(millis() - actionTime < Actions[actionIndex].duration) return;
	stopAction(Actions[actionIndex]);

	actionIndex = (actionIndex + 1) % (sizeof(Actions) / sizeof(Actions[0]));
	doAction(Actions[actionIndex]);

	actionTime = millis();
}

void DemoState::doAction(const DemoState::Action& action){
	if(action.type == Action::Cam){
		cam.setLocally(CameraState{ .Rotation = (uint8_t) action.pos });
	}else if(action.type == Action::Arm){
		arm.setLocally(ArmState{ .Position = (int8_t) action.pos });
	}else if(action.type == Action::Pinch){
		arm.setLocally(ArmState{ .Pinch = (int8_t) action.pos });
	}else if(action.type == Action::Move){
		motors.setLocally(MotorDriveState{ .DriveDirection = { .dir = (uint8_t) action.dir, .speed = 1.0f }});
	}
}

void DemoState::stopAction(const DemoState::Action& action){
	if(action.type == Action::Move){
		motors.setLocally(MotorDriveState{ .DriveDirection = { 0, 0 }});
	}
}
