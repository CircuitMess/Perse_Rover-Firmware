#ifndef PERSE_ROVER_HWVERSION_H
#define PERSE_ROVER_HWVERSION_H

#include <cstdint>
#include <esp_efuse.h>

class HWVersion {
public:
	static bool check();
	static bool write();
	static void log();

	static bool readVersion(uint16_t &version);

	static uint16_t getHardcodedVersion();

private:
	static inline uint16_t CachedVersion = 0;
	/* Curiosity main board. Perseverance boards are fused with 0x0004, so a mismatch here is what
	 * stops this firmware from driving the wrong pinout on the wrong board. */
	static inline constexpr const uint16_t Version = 0x0104;
	/* Blank boards read 0. Fusing happens in the jig test, so allow unfused boards through with a
	 * warning - set this to false to require a fused version before the firmware will run. */
	static inline constexpr const bool AllowUnfused = true;
	static constexpr esp_efuse_desc_t Ver = { EFUSE_BLK3, 16, 16 };
	static constexpr const esp_efuse_desc_t* Efuse_ver[] = { &Ver, nullptr };
};

#endif //PERSE_ROVER_HWVERSION_H