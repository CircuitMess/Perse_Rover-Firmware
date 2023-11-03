#ifndef PERSE_ROVER_WS2812B_H
#define PERSE_ROVER_WS2812B_H

#include <vector>
#include "driver/rmt_encoder.h"
#include <glm.hpp>
#include <driver/rmt_tx.h>

class WS2812B {
public:
	WS2812B(uint32_t numLeds, gpio_num_t gpio);
	virtual ~WS2812B();
	void setAll(glm::vec<3, uint8_t> color);
	void setPixel(glm::vec<3, uint8_t> color, uint32_t index);

	void push();

private:
	static constexpr uint32_t LEDResolution = 10000000;
	const uint32_t NumLeds;
	std::vector<glm::vec<3, uint8_t>> ledPixels;

	rmt_channel_handle_t led_chan = NULL;
	rmt_encoder_handle_t led_encoder = NULL;
	rmt_tx_channel_config_t tx_chan_config;
	rmt_transmit_config_t tx_config;

	typedef struct {
		rmt_encoder_t base;
		rmt_encoder_t* bytes_encoder;
		rmt_encoder_t* copy_encoder;
		int state;
		rmt_symbol_word_t reset_code;
	} rmt_led_strip_encoder_t;

	/**
	 * @brief Create RMT encoder for encoding LED strip pixels into RMT symbols
	 *
	 * @param[in] config Encoder resolution
	 * @param[out] ret_encoder Returned encoder handle
	 * @return
	 *      - ESP_ERR_INVALID_ARG for any invalid arguments
	 *      - ESP_ERR_NO_MEM out of memory when creating led strip encoder
	 *      - ESP_OK if creating encoder successfully
	 */
	static esp_err_t rmt_new_led_strip_encoder(uint32_t resolution, rmt_encoder_handle_t* ret_encoder);
	static size_t
	rmt_encode_led_strip(rmt_encoder_t* encoder, rmt_channel_handle_t channel, const void* primary_data, size_t data_size, rmt_encode_state_t* ret_state);
	static esp_err_t rmt_del_led_strip_encoder(rmt_encoder_t* encoder);
	static esp_err_t rmt_led_strip_encoder_reset(rmt_encoder_t* encoder);
};


#endif //PERSE_ROVER_WS2812B_H
