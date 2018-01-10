/******************************************************************************
 FILE :			slIO.h
 AUTHOR :		Michael Eilers-Smith
 CREATED :		June 2008
 DESCRIPTION :	Interface for the use of the slIO server.
******************************************************************************/

#ifndef _SLIO_H
#define _SLIO_H


#ifndef SLIO_DLL_EXPORT
	#ifdef WIN32
		#define SLIO_DLL_EXPORT __declspec(dllexport)
	#else
		#define SLIO_DLL_EXPORT
	#endif
#endif


#include "slCore.h"
#include "slException.h"


#endif	// _SLIO_H


