#include "Audio.h"
#include "Pins.hpp"
#include "Util/stdafx.h"
#include <driver/i2s.h>
#include <string>
#include "Util/AACDecoder.h"

Audio::Audio(AW9523& aw9523) : Threaded("Audio", 18 * 1024), aw9523(aw9523), playQueue(6){
	const i2s_config_t cfg_i2s = {
			.mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
			.sample_rate = 24000,
			.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
			.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
			.communication_format = I2S_COMM_FORMAT_STAND_I2S,
			.dma_buf_count = 4,
			.dma_buf_len = BufSize,
			.tx_desc_auto_clear = true
	};
	i2s_driver_install(I2S_NUM_0, &cfg_i2s, 0, nullptr);

	const i2s_pin_config_t cfg_i2s_pins = {
			.mck_io_num = -1,
			.bck_io_num = I2S_BCLK,
			.ws_io_num = I2S_LRCLK,
			.data_out_num = I2S_DOUT,
			.data_in_num = -1
	};
	i2s_set_pin(I2S_NUM_0, &cfg_i2s_pins);

	dataBuf.resize(BufSize, 0);

	aw9523.pinMode(EXP_SPKR_EN, AW9523::OUT);
	aw9523.write(EXP_SPKR_EN, true);

	start();
}

Audio::~Audio(){
	Threaded::stop(0);
	playQueue.post(nullptr, portMAX_DELAY);
	while(running()){
		delayMillis(1);
	}
	closeFile();
	i2s_driver_uninstall(Port);
	aw9523.write(EXP_SPKR_EN, false);
}

void Audio::play(const std::string& file){
	if(!enabled) return;

	auto str = std::make_unique<std::string>(file);
	playQueue.post(std::move(str));
}

void Audio::stop(){
	auto str = std::make_unique<std::string>();
	playQueue.post(std::move(str));
}

bool Audio::isEnabled() const{
	return enabled;
}

void Audio::setEnabled(bool enabled){
	Audio::enabled = enabled;
	if(enabled) return;

	stop();
}

const std::string& Audio::getCurrentPlayingFile() const{
	return currentFile;
}

void Audio::loop(){
	if(!aac){
		std::unique_ptr<std::string> queued = playQueue.get(portMAX_DELAY);
		if(queued == nullptr || queued->empty()) return;
		openFile(*queued);
	}

	std::unique_ptr<std::string> queued = playQueue.get(0);
	if(queued){
		if(queued->empty()){
			closeFile();
			return;
		}else{
			openFile(queued->c_str());
		}
	}
	queued.reset();

	if(!aac){
		return;
	}

	const size_t bytesToTransfer = aac->getData(dataBuf.data(), dataBuf.size() * sizeof(int16_t));
	if(bytesToTransfer == 0){
		closeFile();
		return;
	}

	size_t written;
	i2s_write(I2S_NUM_0, dataBuf.data(), bytesToTransfer, &written, portMAX_DELAY);
}

void Audio::openFile(const std::string& file){
	if(file.empty()){
		return;
	}

	closeFile();

	currentFile = file;
	aac = std::make_unique<AACDecoder>(file);
}

void Audio::closeFile(){
	currentFile = "";
	aac.reset();
}
