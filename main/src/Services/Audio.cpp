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

void Audio::play(const std::string& file, bool priority){
	if(!enabled) return;

	auto str = std::make_unique<AudioFile>(file, priority);
	playQueue.post(std::move(str));
}

void Audio::stop(){
	auto str = std::make_unique<AudioFile>();
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
	return currentFile.file;
}

void Audio::loop(){
	if(!aac){
		std::unique_ptr<AudioFile> queued = playQueue.get(portMAX_DELAY);
		if(queued == nullptr || queued->file.empty()) return;
		openFile(*queued);
	}

	std::unique_ptr<AudioFile> queued = playQueue.get(0);
	if(queued){
		if(queued->file.empty()){
			closeFile();
			return;
		}

		if(queued->priority){
			openFile(*queued);
		}else{
			queuedFile = *queued;
		}
	}
	queued.reset();

	if(!aac){
		return;
	}

	const size_t bytesToTransfer = aac->getData(dataBuf.data(), dataBuf.size() * sizeof(int16_t));
	if(bytesToTransfer == 0){
		if(currentFile.state == AudioFile::State::Prefix){
			aac.reset();
			aac = std::make_unique<AACDecoder>(currentFile.file);
			currentFile.state = AudioFile::State::Main;
		}else if(currentFile.state == AudioFile::State::Main){
			aac.reset();

			//to avoid 2 back-to-back beeps when file is queued
			if(!queuedFile.file.empty()){
				closeFile();
				openFile(queuedFile);
				queuedFile = {};
			}else{
				aac = std::make_unique<AACDecoder>(Beeps[rand() % 3]);
				currentFile.state = AudioFile::State::Suffix;
			}
		}else if(currentFile.state == AudioFile::State::Suffix){
			closeFile();

			if(queuedFile.file.empty()) return;

			openFile(queuedFile);
			queuedFile = {};
		}
		return;
	}

	size_t written;
	i2s_write(I2S_NUM_0, dataBuf.data(), bytesToTransfer, &written, portMAX_DELAY);
}

void Audio::openFile(const AudioFile& audioFile){
	if(audioFile.file.empty()){
		return;
	}

	//to avoid 2 back-to-back beeps, if first was interrupted halfway
	bool beepInterrupted = !currentFile.file.empty() && (currentFile.state == AudioFile::State::Prefix || currentFile.state == AudioFile::State::Suffix);
	closeFile();

	currentFile = audioFile;
	std::string path;
	if(currentFile.file == Beeps[0] || currentFile.file == Beeps[1] || currentFile.file == Beeps[2]){
		path = currentFile.file;
		currentFile.state = AudioFile::State::Suffix;
	}else if(beepInterrupted){
		path = currentFile.file;
		currentFile.state = AudioFile::State::Main;
	}else{
		path = Beeps[rand() % 3];
	}
	aac = std::make_unique<AACDecoder>(path);
}

void Audio::closeFile(){
	currentFile.file = "";
	currentFile.priority = false;
	currentFile.state = AudioFile::State::Prefix;
	aac.reset();
}
