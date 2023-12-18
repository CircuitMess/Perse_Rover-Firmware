#ifndef PERSE_ROVER_AUDIO_H
#define PERSE_ROVER_AUDIO_H

#include <string>
#include "Devices/AW9523.h"
#include "Util/Threaded.h"
#include "Util/Queue.h"
#include <driver/i2s_types.h>

class Audio : private Threaded {
public:
	Audio(AW9523& aw9523);
	virtual ~Audio();

	void play(const std::string& file);
	void stop();

	const std::string& getCurrentPlayingFile() const;

private:
	static constexpr i2s_port_t Port = I2S_NUM_0;
	static constexpr size_t BufSize = 1024;

	std::vector<int16_t> dataBuf;

	AW9523& aw9523;

	void loop() override;

	void openFile(const std::string& file);
	void closeFile();

	std::unique_ptr<class AACDecoder> aac;

	PtrQueue<std::string> playQueue;
	std::string currentFile;
};


#endif //PERSE_ROVER_AUDIO_H
