#ifndef PERSE_ROVER_SPIFFS_H
#define PERSE_ROVER_SPIFFS_H


class SPIFFS {
public:
	SPIFFS();
	virtual ~SPIFFS();

private:

	static constexpr char* BasePath = "/spiffs";
	static constexpr char* PartitionLabel = "storage";

};


#endif //PERSE_ROVER_SPIFFS_H
