#include "ADC.h"

static const char* TAG = "ADC";

ADC::ADC(adc_unit_t unit) : unit(unit){
	adc_oneshot_unit_init_cfg_t config = {
			.unit_id = unit,
			.ulp_mode = ADC_ULP_MODE_DISABLE
	};
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&config, &hndl));
}

ADC::~ADC(){
	ESP_ERROR_CHECK(adc_oneshot_del_unit(hndl));
}

void ADC::config(adc_channel_t chan, const adc_oneshot_chan_cfg_t& cfg){
	ESP_ERROR_CHECK(adc_oneshot_config_channel(hndl, chan, &cfg));
}

adc_unit_t ADC::getUnit() const{
	return unit;
}

int ADC::read(adc_channel_t chan){
	int val;
	ESP_ERROR_CHECK(adc_oneshot_read(hndl, chan, &val));
	return val;
}
