#pragma once
#include <afxwin.h>
class Util
{
public:
	Util(void);
	~Util(void);

	static float BytesToFloat(unsigned char *bytes);//convert array of 4 bytes to a floating point number (switches order of bytes)
	static int BytesToInt(unsigned char *bytes);//convert array of 4 bytes to an integer number (switches order of bytes)
};
