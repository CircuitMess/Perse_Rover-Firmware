#ifndef PERSE_ROVER_TCPSERVER_H
#define PERSE_ROVER_TCPSERVER_H

#include <cstdint>
#include <cstddef>

class TCPServer {
public:
	TCPServer();

	bool isConnected();

	bool accept();
	void disconnect();

	bool read(uint8_t* buf, size_t count);
	bool write(uint8_t* data, size_t count);

private:
	int sock = -1;

	int client = -1;

};


#endif //PERSE_ROVER_TCPSERVER_H
