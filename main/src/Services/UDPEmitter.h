#ifndef PERSE_ROVER_UDPEMITTER_H
#define PERSE_ROVER_UDPEMITTER_H

#include <cstdint>
#include <cstddef>
#include <lwip/sockets.h>

class UDPEmitter {
public:
	UDPEmitter();
	virtual ~UDPEmitter();

	bool write(uint8_t* data, size_t count);

private:
	int sock = -1;

	sockaddr_in dest{};

};


#endif //PERSE_ROVER_UDPEMITTER_H
