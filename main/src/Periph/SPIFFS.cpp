#include "SPIFFS.h"
#include <esp_spiffs.h>
#include <esp_log.h>

static const char* TAG = "SPIFFS";

SPIFFS::SPIFFS(){
	esp_vfs_spiffs_conf_t conf = {
			.base_path = BasePath,
			.partition_label = PartitionLabel,
			.max_files = 8,
			.format_if_mount_failed = false
	};

	auto ret = esp_vfs_spiffs_register(&conf);
	if(ret != ESP_OK){
		if(ret == ESP_FAIL){
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		}else if(ret == ESP_ERR_NOT_FOUND){
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		}else{
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
	}
}

SPIFFS::~SPIFFS(){
	esp_vfs_spiffs_unregister(PartitionLabel);
}

