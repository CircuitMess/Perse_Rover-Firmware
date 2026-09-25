#ifndef PERSE_ROVER_AUDIOTESTSTATE_H
#define PERSE_ROVER_AUDIOTESTSTATE_H

#include "Services/StateMachine.h"
#include "Util/Events.h"

/**
 * Listening test for the Curiosity voice candidates. Every press of the power button plays the next
 * sample in Samples (bare, without the Beep prefix and suffix) and logs its path, so whoever is
 * listening can tell which take they are hearing. A press during playback skips to the next sample.
 * Holding the button is left to PowerButtonService, which turns the board off.
 */
class AudioTestState : public State {
public:
	AudioTestState();
	~AudioTestState() override;

	void loop() override;
	void unblock() override;

private:
	static constexpr const char* Samples[] = {
			"/spiffs/Test/set2C/Intro.aac", "/spiffs/Test/set2C/PowerOn.aac",
			"/spiffs/Test/set3B/Intro.aac", "/spiffs/Test/set3B/PowerOn.aac",
			"/spiffs/Test/set4C/Intro.aac", "/spiffs/Test/set4C/PowerOn.aac",
			"/spiffs/Test/set4lib/Intro.aac", "/spiffs/Test/set4lib/PowerOn.aac",
			"/spiffs/Test/set5B/Intro.aac", "/spiffs/Test/set5B/PowerOn.aac",
			"/spiffs/Test/set7A/Intro.aac", "/spiffs/Test/set7A/PowerOn.aac"
	};
	static constexpr size_t SampleCount = sizeof(Samples) / sizeof(Samples[0]);

	EventQueue evts;
	size_t index = SampleCount - 1; // so the first press plays Samples[0]

	void playNext();
};

#endif //PERSE_ROVER_AUDIOTESTSTATE_H
