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
		//Only a TCP connected event will unblock the thread
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
	}

	return e;
}
