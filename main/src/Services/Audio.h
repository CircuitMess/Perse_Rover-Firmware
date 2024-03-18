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

	void play(const std::string& file, bool priority = false);
	void stop();

	bool isEnabled() const;
	void setEnabled(bool enabled);

	const std::string& getCurrentPlayingFile() const;

private:
	static constexpr i2s_port_t Port = I2S_NUM_0;
	static constexpr size_t BufSize = 1024;

	struct AudioFile {
		std::string file;
		bool priority;
	};

	bool enabled = true;

	std::vector<int16_t> dataBuf;

	AW9523& aw9523;

	void loop() override;

	void openFile(const AudioFile& audioFile);
	void closeFile();

	std::unique_ptr<class AACDecoder> aac;

	PtrQueue<AudioFile> playQueue;
	AudioFile currentFile;
	AudioFile queuedFile;
};


#endif //PERSE_ROVER_AUDIO_H
