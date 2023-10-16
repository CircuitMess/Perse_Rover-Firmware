#include "Comm.h"
#include "Util/Services.h"

Comm::Comm() : Threaded("Comm", 4 * 1024), tcp(*(TCPServer*) Services.get(Service::TCP)), queue(10){
	Events::listen(Facility::TCP, &queue);
	start();
}

Comm::~Comm(){
	Events::unlisten(&queue);
	stop();
}

void Comm::sendHeadlightsState(HeadlightsMode headlights) {
	const ControlPacket packet = {
			.type = CommType::Headlights,
			.data = (uint8_t)headlights
	};

	sendPacket(packet);
}

void Comm::sendArmPositionState(ArmPos position) {
	const ControlPacket packet = {
			.type = CommType::Headlights,
			.data = (uint8_t)position
	};

	sendPacket(packet);
}

void Comm::sendArmPinchState(ArmPinch pinch) {
	const ControlPacket packet = {
			.type = CommType::Headlights,
			.data = (uint8_t)(pinch)
	};

	sendPacket(packet);
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

	switch(packet.type) {
		case CommType::DriveDir: {
			e.dir = CommData::decodeDriveDir(packet.data);
			break;
		}
		case CommType::Headlights: {
			e.headlights = packet.data > 0 ? HeadlightsMode::On : HeadlightsMode::Off;
			break;
		}
		case CommType::ArmPosition: {
			e.armPos = (ArmPos)packet.data;
			e.armPinch = -1;
			break;
		}
		case CommType::ArmPinch: {
			e.armPos = -1;
			e.armPinch = (ArmPinch)packet.data;
		}
		default: {
			break;
		}
	}

	return e;
}
