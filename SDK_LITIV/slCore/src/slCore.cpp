#include "slCore.h"


#ifdef WIN32
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	return TRUE;
}
#endif	// WIN32


slImage1ch grayClone(const slImage3ch &image, typeColorSys csys)
{
	slImage1ch tmp;

	if (csys == SL_BGR) {
		// BGR to gray
		cvtColor(image, tmp, CV_BGR2GRAY);
	}
	else {
		// HSV to V (gray)
		slImage1ch hsv[3];
		split(image, &hsv[0]);
		tmp = hsv[2];
	}

	return tmp;
}


