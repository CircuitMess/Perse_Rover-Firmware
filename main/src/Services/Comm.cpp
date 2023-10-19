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

void Comm::sendModulePlug(ModuleType type, ModuleBus bus, bool insert){
	if(!modulesEnabled) return;

	uint8_t data = CommData::encodeModulePlug({ type, bus, insert });
	ControlPacket packet{ CommType::ModulePlug, data };
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
		modulesEnabled = false; //reset modulesEnable on disconnect
		::Event event{};
		while(!queue.get(event, portMAX_DELAY));
		free(event.data);
	}
}

Comm::Event Comm::processPacket(const ControlPacket& packet){
	Event e{};
	e.raw = packet.data;
	e.type = packet.type;
	switch(packet.type){
		case CommType::DriveDir:
			e.dir = CommData::decodeDriveDir(packet.data);
			break;
		case CommType::ModulePlug:
			break;
		case CommType::ModuleData:
			break;
		case CommType::ModulesEnable:
			modulesEnabled = packet.data;
			break;
		case CommType::Headlights:
			break;
	}

	return e;
}

void Comm::sendModuleData(ModuleData data){
	if(!modulesEnabled) return;

	if(!tcp.isConnected()) return;

	auto type = CommType::ModuleData;
	tcp.write((uint8_t*) &type, sizeof(CommType));
	tcp.write((uint8_t*) &data, sizeof(ModuleData));
}
