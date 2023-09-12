#include "WiFiAP.h"
#include "Util/Events.h"
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_wifi_default.h>
#include <string>
#include <cstring>
#include <esp_log.h>

static const char* TAG = "WiFi_AP";

static std::string mac2str(uint8_t ar[]){
	std::string str;
	for(int i = 0; i < 6; ++i){
		char buf[3];
		sprintf(buf, "%02X", ar[i]);
		str += buf;
		if(i < 5) str += ':';
	}
	return str;
}

WiFiAP::WiFiAP(){
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, [](void* arg, esp_event_base_t base, int32_t id, void* data){
		if(base != WIFI_EVENT) return;
		auto wifi = static_cast<WiFiAP*>(arg);
		wifi->event(id, data);
	}, this, &evtHandler);

	ESP_ERROR_CHECK(esp_netif_init());
	esp_netif_create_default_wifi_ap();

	wifi_init_config_t cfg_wifi = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg_wifi);

	wifi_config_t cfg_ap = {
			.ap = {
					.ssid = "Perseverance Rover",
					.password = "12345678",
					.channel = 0,
					.authmode = WIFI_AUTH_WPA2_PSK,
					.ssid_hidden = true,
					.max_connection = 1,
					.pmf_cfg = {
							.required = true,
					},
			},
	};
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &cfg_ap));
	ESP_ERROR_CHECK(esp_wifi_start());
}

void WiFiAP::event(int32_t id, void* data){
	ESP_LOGD(TAG, "Evt %ld", id);

	if(id == WIFI_EVENT_AP_STACONNECTED){
		auto event = (wifi_event_ap_staconnected_t*) data;
		const auto mac = mac2str(event->mac);
		ESP_LOGI(TAG, "station %s join, AID=%d", mac.c_str(), event->aid);

		Event evt { .action = Event::Connect };
		memcpy(evt.connect.mac, event->mac, 6);
		Events::post(Facility::WiFiAP, evt);
	}else if(id == WIFI_EVENT_AP_STADISCONNECTED){
		auto event = (wifi_event_ap_stadisconnected_t*) data;
		const auto mac = mac2str(event->mac);
		ESP_LOGI(TAG, "station %s leave, AID=%d", mac.c_str(), event->aid);

		Event evt { .action = Event::Disconnect };
		memcpy(evt.disconnect.mac, event->mac, 6);
		Events::post(Facility::WiFiAP, evt);
	}
}

