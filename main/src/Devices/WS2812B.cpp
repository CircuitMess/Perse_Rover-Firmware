#include "WS2812B.h"
#include <driver/rmt_tx.h>
#include "esp_check.h"

static const char* TAG = "led_encoder";


WS2812B::WS2812B(uint32_t numLeds, gpio_num_t gpio) : NumLeds(numLeds), ledPixels(NumLeds, { 0, 0, 0 }){

	tx_chan_config = {
			.gpio_num = gpio,
			.clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
			.resolution_hz = LEDResolution,
			.mem_block_symbols = 64, // increase the block size can make the LED less flickering
			.trans_queue_depth = 4, // set the number of transactions that can be pending in the background
	};
	ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

	ESP_ERROR_CHECK(rmt_new_led_strip_encoder(LEDResolution, &led_encoder));

	ESP_ERROR_CHECK(rmt_enable(led_chan));

	tx_config = {
			.loop_count = 0, // no transfer loop
	};
	push();
}

WS2812B::~WS2812B(){
	rmt_disable(led_chan);
	if(led_encoder){
		rmt_del_encoder(led_encoder);
	}
	rmt_del_channel(led_chan);
}

void WS2812B::setAll(glm::vec<3, uint8_t> color){
	std::fill(ledPixels.begin(), ledPixels.end(), color);
}

void WS2812B::setPixel(glm::vec<3, uint8_t> color, uint32_t index){
	ledPixels[index] = color;
}

void WS2812B::push(){
	ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, &ledPixels[0], NumLeds * 3, &tx_config));
}

size_t
WS2812B::rmt_encode_led_strip(rmt_encoder_t* encoder, rmt_channel_handle_t channel, const void* primary_data, size_t data_size, rmt_encode_state_t* ret_state){
	rmt_led_strip_encoder_t* led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
	rmt_encoder_handle_t bytes_encoder = led_encoder->bytes_encoder;
	rmt_encoder_handle_t copy_encoder = led_encoder->copy_encoder;
	rmt_encode_state_t session_state = RMT_ENCODING_RESET;
	rmt_encode_state_t state = RMT_ENCODING_RESET;
	size_t encoded_symbols = 0;
	switch(led_encoder->state){
		case 0: // send RGB data
			encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
			if(session_state & RMT_ENCODING_COMPLETE){
				led_encoder->state = 1; // switch to next state when current encoding session finished
			}
			if(session_state & RMT_ENCODING_MEM_FULL){
				state = (rmt_encode_state_t) (state | RMT_ENCODING_MEM_FULL);
				goto out; // yield if there's no free space for encoding artifacts
			}
			// fall-through
		case 1: // send reset code
			encoded_symbols += copy_encoder->encode(copy_encoder, channel, &led_encoder->reset_code,
													sizeof(led_encoder->reset_code), &session_state);
			if(session_state & RMT_ENCODING_COMPLETE){
				led_encoder->state = RMT_ENCODING_RESET; // back to the initial encoding session
				state = (rmt_encode_state_t) (state | RMT_ENCODING_COMPLETE);
			}
			if(session_state & RMT_ENCODING_MEM_FULL){
				state = (rmt_encode_state_t) (state | RMT_ENCODING_MEM_FULL);
				goto out; // yield if there's no free space for encoding artifacts
			}
	}
	out:
	*ret_state = state;
	return encoded_symbols;
}

esp_err_t WS2812B::rmt_del_led_strip_encoder(rmt_encoder_t* encoder){
	rmt_led_strip_encoder_t* led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
	rmt_del_encoder(led_encoder->bytes_encoder);
	rmt_del_encoder(led_encoder->copy_encoder);
	free(led_encoder);
	return ESP_OK;
}

esp_err_t WS2812B::rmt_led_strip_encoder_reset(rmt_encoder_t* encoder){
	rmt_led_strip_encoder_t* led_encoder = __containerof(encoder, rmt_led_strip_encoder_t, base);
	rmt_encoder_reset(led_encoder->bytes_encoder);
	rmt_encoder_reset(led_encoder->copy_encoder);
	led_encoder->state = RMT_ENCODING_RESET;
	return ESP_OK;
}

esp_err_t WS2812B::rmt_new_led_strip_encoder(uint32_t resolution, rmt_encoder_handle_t* ret_encoder){
	esp_err_t ret = ESP_OK;
	uint32_t reset_ticks = resolution / 1000000 * 50 / 2; // reset code duration defaults to 50us
	rmt_bytes_encoder_config_t bytes_encoder_config = {
			.bit0 = {
					.duration0 = (uint16_t) (T0H * resolution / 1000000),
					.level0 = 1,
					.duration1 = (uint16_t) (T0L * resolution / 1000000),
					.level1 = 0
			},
			.bit1 = {
					.duration0 = (uint16_t) (T1H * resolution / 1000000),
					.level0 = 1,
					.duration1 = (uint16_t) (T1L * resolution / 1000000),
					.level1 = 0
			},
			.flags = { .msb_first = 1 } // WS2812 transfer bit order: G7...G0R7...R0B7...B0
	};
	rmt_copy_encoder_config_t copy_encoder_config = {};
	rmt_led_strip_encoder_t* led_encoder = nullptr;
	ESP_GOTO_ON_FALSE(ret_encoder, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
	led_encoder = (rmt_led_strip_encoder_t*) calloc(1, sizeof(rmt_led_strip_encoder_t));
	ESP_GOTO_ON_FALSE(led_encoder, ESP_ERR_NO_MEM, err, TAG, "no mem for led strip encoder");
	led_encoder->base.encode = rmt_encode_led_strip;
	led_encoder->base.del = rmt_del_led_strip_encoder;
	led_encoder->base.reset = rmt_led_strip_encoder_reset;
	// different led strip might have its own timing requirements, following parameter is for WS2812

	ESP_GOTO_ON_ERROR(rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder->bytes_encoder), err, TAG, "create bytes encoder failed");
	ESP_GOTO_ON_ERROR(rmt_new_copy_encoder(&copy_encoder_config, &led_encoder->copy_encoder), err, TAG, "create copy encoder failed");

	led_encoder->reset_code = (rmt_symbol_word_t) {
			.duration0 = (uint16_t) reset_ticks,
			.level0 = 0,
			.duration1 = (uint16_t) reset_ticks,
			.level1 = 0
	};
	*ret_encoder = &(led_encoder->base);
	return ESP_OK;


	err:
	if(led_encoder){
		if(led_encoder->bytes_encoder){
			rmt_del_encoder(led_encoder->bytes_encoder);
		}
		if(led_encoder->copy_encoder){
			rmt_del_encoder(led_encoder->copy_encoder);
		}
		free(led_encoder);
	}
	return ret;
}


