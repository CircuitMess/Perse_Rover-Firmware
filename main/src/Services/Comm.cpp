#include "Comm.h"
#include <RoverStateUtil.h>
#include "Util/Services.h"

Comm::Comm() : Threaded("Comm", 4 * 1024), tcp(*(TCPServer*) Services.get(Service::TCP)), queue(10){
	Events::listen(Facility::TCP, &queue);
	start();
}

Comm::~Comm(){
	Events::unlisten(&queue);
	stop();
}

void Comm::sendHeadlightsState(HeadlightsMode headlights, bool local){
	const ControlPacket packet = {
			.type = CommType::Headlights,
			.data = encodeRoverState((uint8_t) headlights, local)
	};

	sendPacket(packet);
}

void Comm::sendArmPositionState(ArmPos position, bool local){
	const ControlPacket packet = {
			.type = CommType::ArmPosition,
			.data = encodeRoverState((uint8_t) position, local)
	};

	sendPacket(packet);
}

void Comm::sendArmPinchState(ArmPinch pinch, bool local){
	const ControlPacket packet = {
			.type = CommType::ArmPinch,
			.data = encodeRoverState((uint8_t) pinch, local)
	};

	sendPacket(packet);
}

void Comm::sendCameraState(CameraRotation rotation, bool local){
	const ControlPacket packet = {
			.type = CommType::CameraRotation,
			.data = encodeRoverState(rotation, local)
	};

	sendPacket(packet);
}

void Comm::sendBattery(uint8_t batteryPercent){
	const ControlPacket packet = {
			.type = CommType::Battery,
			.data = batteryPercent
	};

	sendPacket(packet);
}

void Comm::sendModulePlug(ModuleType type, ModuleBus bus, bool insert){
	const uint8_t data = CommData::encodeModulePlug({ type, bus, insert });

	const ControlPacket packet{
		CommType::ModulePlug,
		data
	};

	sendPacket(packet);
}

void Comm::sendModuleData(ModuleData data){
	if(!tcp.isConnected()) return;

	auto type = CommType::ModuleData;
	tcp.write((uint8_t*) &type, sizeof(CommType));
	tcp.write((uint8_t*) &data, sizeof(ModuleData));
}

void Comm::sendPacket(const ControlPacket& packet){
	if(!tcp.isConnected()) return;

	tcp.write((uint8_t*) &packet, sizeof(ControlPacket));
}

void Comm::loop(){
	bool readOK = false;
	if(tcp.isConnected()){
		ControlPacket packet{};
		readOK = tcp.read(reinterpret_cast<uint8_t*>(&packet), sizeof(ControlPacket));

		if(readOK){
			Event e = processPacket(packet);
			Events::post(Facility::Comm, e);
		}
	}

	if(!tcp.isConnected() || !readOK){
		::Event event{};
		while(!queue.get(event, portMAX_DELAY));
		free(event.data);
	}
}

Comm::Event Comm::processPacket(const ControlPacket& packet){
	Event e{
			.type = packet.type,
			.raw = packet.data
	};

	switch(packet.type){
		case CommType::DriveDir:{
			e.dir = CommData::decodeDriveDir(packet.data);
			break;
		}
		case CommType::Headlights:{
			e.headlights = packet.data > 0 ? HeadlightsMode::On : HeadlightsMode::Off;
			break;
		}
		case CommType::ArmPosition:{
			e.armPos = (ArmPos) packet.data;
			e.armPinch = -1;
			break;
		}
		case CommType::ArmPinch:{
			e.armPos = -1;
			e.armPinch = (ArmPinch) packet.data;
			break;
		}
		case CommType::CameraRotation:{
			e.cameraRotation = packet.data;
			break;
		}
		case CommType::FeedQuality:{
			e.feedQuality = packet.data;
			break;
		}
		case CommType::ScanMarkers: {
			e.scanningEnable = packet.data;
			break;
		}
		case CommType::Emergency:{
			e.emergency = (bool)packet.data;
			break;
		}
		case CommType::Audio:{
			e.audio = (bool)packet.data;
			break;
		}
		case CommType::ModulePlug:
		case CommType::ModuleData:
		case CommType::ModulesEnable:
		default: {
			break;
		}
	}

	return e;
}
