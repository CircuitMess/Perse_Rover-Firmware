#ifndef CLOCKSTAR_FIRMWARE_ADC_H
#define CLOCKSTAR_FIRMWARE_ADC_H

#include <esp_adc/adc_oneshot.h>

class ADC {
public:
	explicit ADC(adc_unit_t unit);
	virtual ~ADC();

	adc_unit_t getUnit() const;

	void config(adc_channel_t chan, const adc_oneshot_chan_cfg_t& cfg);

	int read(adc_channel_t chan);

private:
	adc_oneshot_unit_handle_t hndl;
	adc_unit_t unit;

};

#endif //CLOCKSTAR_FIRMWARE_ADC_H
