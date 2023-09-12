#ifndef PERSE_ROVER_WIFIAP_H
#define PERSE_ROVER_WIFIAP_H

#include <esp_event.h>

class WiFiAP {
public:
	WiFiAP();

	struct Event {
		enum { Connect, Disconnect } action;
		union {
			struct {
				uint8_t mac[6];
			} connect;

			struct {
				uint8_t mac[6];
			} disconnect;
		};
	};

private:
	esp_event_handler_instance_t evtHandler;
	void event(int32_t id, void* data);

	void setHidden(bool hide);

};


#endif //PERSE_ROVER_WIFIAP_H
