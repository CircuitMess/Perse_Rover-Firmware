#include "TCPServer.h"
#include "Util/Events.h"
#include <lwip/sockets.h>
#include <esp_log.h>

static const char* TAG = "TCPServer";

TCPServer::TCPServer(){
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if(sock == -1){
		ESP_LOGE(TAG, "Can't create socket, errno=%d: %s", errno, strerror(errno));
		return;
	}

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6000);
	inet_pton(AF_INET, "11.0.0.1", &addr.sin_addr);
	if(bind(sock, (sockaddr*) &addr, sizeof(addr)) != 0){
		ESP_LOGE(TAG, "Can't bind address to socket, errno=%d: %s", errno, strerror(errno));
		sock = -1;
		return;
	}

	if(listen(sock, 1) != 0){
		ESP_LOGE(TAG, "Can't listen on socket, errno=%d: %s", errno, strerror(errno));
		sock = -1;
		return;
	}

	fcntl(sock, F_SETFL, O_NONBLOCK);
}

bool TCPServer::isConnected(){
	return client != -1;
}

bool TCPServer::accept(){
	if(sock == -1){
		ESP_LOGE(TAG, "Accept, but sock isn't set up");
		return false;
	}

	if(client != -1){
		ESP_LOGE(TAG, "Accept, but client already connected");
		return false;
	}

	sockaddr_in addr_client{};
	socklen_t addr_size = sizeof(addr_client);
	if((client = ::accept(sock, (sockaddr*) &addr_client, &addr_size)) == -1){
		ESP_LOGV(TAG, "Can't accept, errno=%d: %s", errno, strerror(errno));
		return false;
	}

	int keepAlive = 1;
	int keepIdle = 4; // Time (in seconds) without packets before keep-alive packet sending begins
	int keepInterval = 2; // Interval between keep-alive packets
	int keepCount = 2; // Number of keep-alive packets before connection drops
	setsockopt(client, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
	setsockopt(client, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
	setsockopt(client, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
	setsockopt(client, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

	char addr_str[32];
	inet_ntoa_r(addr_client.sin_addr, addr_str, sizeof(addr_str)-1);

	ESP_LOGI(TAG, "Client %s connected", addr_str);

	Event event{ Event::Status::Connected };
	Events::post(Facility::TCP, event);

	return true;
}

void TCPServer::disconnect(){
	if(client == -1){
		ESP_LOGW(TAG, "Disconnect, but client isn't connected");
		return;
	}

	close(client);
	client = -1;

	Event event{ Event::Status::Disconnected };
	Events::post(Facility::TCP, event);
}

bool TCPServer::read(uint8_t* buf, size_t count){
	if(client == -1){
		ESP_LOGW(TAG, "Read, but client isn't connected");
		return false;
	}

	if(count == 0) return true;

	size_t total = 0;
	while(total < count){
		int now = ::read(client, buf + total, count - total);

		if(now == 0){
			disconnect();
			return false;
		}else if(now < 0){
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				vTaskDelay(1);
				continue;
			}else{
				disconnect();
				return false;
			}
		}

		total += now;
	}

	return true;
}

bool TCPServer::write(uint8_t* data, size_t count){
	if(client == -1){
		ESP_LOGW(TAG, "Write, but client isn't connected");
		return false;
	}

	if(count == 0) return true;

	size_t total = 0;
	while(total < count){
		int now = ::write(client, data + total, count - total);

		if(now == 0){
			disconnect();
			return false;
		}else if(now < 0){
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				vTaskDelay(1);
				continue;
			}else{
				disconnect();
				return false;
			}
		}

		total += now;
	}

	return true;
}
