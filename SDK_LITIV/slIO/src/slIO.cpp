/******************************************************************************
 * FILE :	      slIO.cpp
 * DESCRIPTION :	This is the implementation of the interface for the use of 
                  the slIO server.
 * AUTHORS :      Michael Eilers-Smith
 * DATE :         June 2008
******************************************************************************/

#include "slIO.h"


#ifdef WIN32
#include <windows.h>
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
	return TRUE;
}
#endif	// WIN32


