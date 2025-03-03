#ifndef SPIFFSCHECKSUM_HPP
#define SPIFFSCHECKSUM_HPP

struct {
	const char* name;
	uint32_t sum;
} static const SPIFFSChecksums[] = {
	{ "/spiffs/Beep.aac", 312585},
	{ "/spiffs/Beep2.aac", 304054},
	{ "/spiffs/Beep3.aac", 307052},
	{ "/spiffs/EasterEggs/Random1.aac", 1494827},
	{ "/spiffs/EasterEggs/Random2.aac", 1705068},
	{ "/spiffs/EasterEggs/Random3.aac", 1451374},
	{ "/spiffs/EasterEggs/Random4.aac", 1434439},
	{ "/spiffs/EasterEggs/Random5.aac", 1550121},
	{ "/spiffs/General/BattEmptyCtrl.aac", 1916620},
	{ "/spiffs/General/BattEmptyRover.aac", 1658546},
	{ "/spiffs/General/BattLowRover.aac", 2054091},
	{ "/spiffs/General/CamFail.aac", 1528448},
	{ "/spiffs/General/PairFail.aac", 2049426},
	{ "/spiffs/General/PairStart.aac", 1881009},
	{ "/spiffs/General/PairSuccess.aac", 1827694},
	{ "/spiffs/General/PowerOn.aac", 1110982},
	{ "/spiffs/General/SignalLost.aac", 1431508},
	{ "/spiffs/General/SignalWeak.aac", 3566650},
	{ "/spiffs/Markers/Advancing.aac", 993199},
	{ "/spiffs/Markers/Alert.aac", 2319082},
	{ "/spiffs/Markers/Ingenuity.aac", 2277302},
	{ "/spiffs/Markers/LeftForward.aac", 1095938},
	{ "/spiffs/Markers/Life.aac", 1362875},
	{ "/spiffs/Markers/Rende.aac", 2470966},
	{ "/spiffs/Markers/RightForward.aac", 1192027},
	{ "/spiffs/Markers/Rotate.aac", 885660},
	{ "/spiffs/Markers/Samples.aac", 1837410},
	{ "/spiffs/Markers/Scouting.aac", 554964},
	{ "/spiffs/Markers/TakingSample.aac", 976956},
	{ "/spiffs/Modules/AirBad.aac", 2312611},
	{ "/spiffs/Modules/AirOff.aac", 1987785},
	{ "/spiffs/Modules/AirOn.aac", 1895356},
	{ "/spiffs/Modules/AltiOff.aac", 2539329},
	{ "/spiffs/Modules/AltiOn.aac", 2347774},
	{ "/spiffs/Modules/GyroOff.aac", 1262949},
	{ "/spiffs/Modules/GyroOn.aac", 1009243},
	{ "/spiffs/Modules/GyroTilt.aac", 1813433},
	{ "/spiffs/Modules/LedOff.aac", 1468299},
	{ "/spiffs/Modules/LedOn.aac", 1473024},
	{ "/spiffs/Modules/LightSensOff.aac", 1448661},
	{ "/spiffs/Modules/LightSensOn.aac", 1367603},
	{ "/spiffs/Modules/MotionDetect.aac", 1511956},
	{ "/spiffs/Modules/MotionOff.aac", 1675463},
	{ "/spiffs/Modules/MotionOn.aac", 1507280},
	{ "/spiffs/Modules/RgbOff.aac", 1714454},
	{ "/spiffs/Modules/RgbOn.aac", 1570641},
	{ "/spiffs/Modules/TempOff.aac", 2248274},
	{ "/spiffs/Modules/TempOn.aac", 2057001},
	{ "/spiffs/Systems/ArmOff.aac", 1068200},
	{ "/spiffs/Systems/ArmOn.aac", 808433},
	{ "/spiffs/Systems/LightOff.aac", 1116019},
	{ "/spiffs/Systems/LightOn.aac", 887119},
	{ "/spiffs/Systems/PanicOff.aac", 2330457},
	{ "/spiffs/Systems/PanicOn.aac", 2004366},
	{ "/spiffs/Systems/ScanOff.aac", 1192538},
	{ "/spiffs/Systems/ScanOn.aac", 942646}
};

#endif //SPIFFSCHECKSUM_HPP
