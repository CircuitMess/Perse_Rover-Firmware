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
	createNetif();

	wifi_init_config_t cfg_wifi = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg_wifi);

	wifi_config_t cfg_ap = {
			.ap = {
					.password = "RoverRover",
					.channel = 1,
					.authmode = WIFI_AUTH_WPA2_PSK,
					.ssid_hidden = false,
					.max_connection = 1
			},
	};

	uint32_t randID = rand() % 1000000;
	std::string ssid = "Perseverance Rovee #" + std::to_string(randID);
	strcpy((char*) cfg_ap.ap.ssid, ssid.c_str());

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &cfg_ap));
	ESP_ERROR_CHECK(esp_wifi_start());
}

void WiFiAP::setHidden(bool hidden){
	return;

	wifi_config_t config;
	esp_wifi_get_config(WIFI_IF_AP, &config);
	config.ap.ssid_hidden = hidden;
	esp_wifi_set_config(WIFI_IF_AP, &config);
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

esp_netif_t* WiFiAP::createNetif(){
	esp_netif_inherent_config_t base{};
	memcpy(&base, ESP_NETIF_BASE_DEFAULT_WIFI_AP, sizeof(esp_netif_inherent_config_t));
	base.flags = (esp_netif_flags_t) ((base.flags & ~(ESP_NETIF_DHCP_SERVER | ESP_NETIF_DHCP_CLIENT | ESP_NETIF_FLAG_EVENT_IP_MODIFIED)) | ESP_NETIF_FLAG_GARP);

	const esp_netif_ip_info_t ip = {
			.ip =		{ .addr = esp_ip4addr_aton("11.0.0.1") },
			.netmask =	{ .addr = esp_ip4addr_aton("255.255.255.0") },
			.gw =		{ .addr = esp_ip4addr_aton("11.0.0.1") },
	};
	base.ip_info = &ip;

	esp_netif_config_t cfg = ESP_NETIF_DEFAULT_WIFI_AP();
	cfg.base = &base;

	esp_netif_t* netif = esp_netif_new(&cfg);
	assert(netif);
	esp_netif_set_default_netif(netif);

	esp_netif_attach_wifi_ap(netif);
	esp_wifi_set_default_wifi_ap_handlers();

	return netif;
}
