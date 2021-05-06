#include "pch.h"
#include "Util.h"

Util::Util(void)
{
}

Util::~Util(void)
{
}

/**
 * @brief convert array of 4 bytes to a floating point number (switches order of bytes)
 * 
 * @param bytes bytes from source that has their order switched from the order that Windows expects for memory conversion to floating point
 * @return float the floating point value represented by the 4 bytes pointed to by bytes
 */
float Util::BytesToFloat(unsigned char *bytes) {
	float fRetval = 0;
	memcpy(&fRetval,bytes,sizeof(float));
	return fRetval;
}

/**
 * @brief convert array of 4 bytes to an integer number (switches order of bytes)
 * 
 * @param bytes bytes from source that has their order switched from the order that Windows expects for memory conversion to integer
 * @return int the integer value represented by the 4 bytes pointed to by bytes
 */
int Util::BytesToInt(unsigned char *bytes) {
	int nRetval = 0;
	memcpy(&nRetval,bytes,sizeof(int));
	return nRetval;
}