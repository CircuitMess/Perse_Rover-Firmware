#ifndef PERSE_ROVER_WIFIAP_H
#define PERSE_ROVER_WIFIAP_H

#include <esp_event.h>
#include <esp_netif_types.h>

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

	/**
	 * Set SSID broadcast on or off.
	 * @param hidden True - hide SSID (default), False - broadcast SSID
	 */
	void setHidden(bool hidden);

private:
	esp_event_handler_instance_t evtHandler;
	void event(int32_t id, void* data);

	static esp_netif_t* createNetif();

};


#endif //PERSE_ROVER_WIFIAP_H
