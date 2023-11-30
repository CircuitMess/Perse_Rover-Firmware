#include "UDPEmitter.h"
#include <lwip/sockets.h>
#include <esp_log.h>

static const char* TAG = "UDPEmitter";

UDPEmitter::UDPEmitter(){
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(sock == -1){
		ESP_LOGE(TAG, "Can't create socket, errno=%d: %s", errno, strerror(errno));
		return;
	}

	dest.sin_family = AF_INET;
	dest.sin_port = htons(6001);
	inet_pton(AF_INET, "11.0.0.2", &dest.sin_addr);
}

UDPEmitter::~UDPEmitter(){
	close(sock);
}

bool UDPEmitter::write(uint8_t* data, size_t count){
	if(sock == -1){
		ESP_LOGW(TAG, "Write, but socket not set-up");
		return false;
	}

	if(count == 0) return true;

	size_t total = 0;
	while(total < count){
		const int now = ::sendto(sock, data + total, count - total, 0, (sockaddr*) &dest, sizeof(dest));

		if(now == 0){
			return false;
		}else if(now < 0){
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				vTaskDelay(1);
				continue;
			}else{
				return false;
			}
		}

		total += now;
	}

	return true;
}
