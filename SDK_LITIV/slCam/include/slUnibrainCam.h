/*!	\file	slUnibrainCam.h
 *	\brief	This file contains the class slUnibrainCam.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		May 2011
 */

#ifndef SLUNIBRAINCAM_H
#define SLUNIBRAINCAM_H


#include <set>
#include <windows.h>

#include <Firei.h>
#include <ubui.h>
#if defined(MIN) || defined(MAX)
#undef MIN
#undef MAX
#endif

#include "slCamIn.h"


//!	This class uses Unibrain's SDK for the use of Sony IIDC cameras
/*!
 *	This class is compatible with 1 channel and 3 channels images.
 *	In case of 1 channel format from camera, slUnibrainCam will ask
 *	if user wants a conversion with a Bayer filter.
 *	Then, the output will be 3 channels images.
 *
 *	The open() function usually takes an empty string.  A non empty string
 *	could possibly contain the GUID code, but it is not yet supported.
 *	With the empty guid string, a window could appear to let the user choose
 *	the camera he wants to use.  If only one Unibrain compatible camera is
 *	connected to the computer, no window is shown, and this camera is chosen by default.
 *
 *	\see		slCamIn
 *	\author		Pier-Luc St-Onge
 *	\date		May 2011
 */
class SLCAMIN_DLL_EXPORT slUnibrainCam: public slCamIn
{
public:
	slUnibrainCam();			//!< Constructor
	virtual ~slUnibrainCam();

	slUnibrainCam& open(const std::string &guid = "");	//!< Empty guid lets you choose a camera from a GUI.
	void close();										//!< Close camera connexion

	std::string description2() const;	//!< Other specifications
	bool inColors() const;				//!< Useful to know if the output format is 1 or 3 channels

	bool hasParameters() const;			//!< Returns true: GUI available
	bool hasDynamicParameters() const;	//!< Returns true: GUI available at capture time
	void showParameters();				//!< Show parameters editor
	void hideParameters();				//!< Hide parameters editor

	void read(slMat &image);			//!< Capture an image from camera and copy header to image

protected:
	void startAfter();
	void stopBefore();

private:
	void iidcInit();
	void iidcUninit();

	void obtainSpecifications();
	bool isStarted() const;

	static FIREi_CAMERA_GUID invalidGUID();

	static long long compare(const FIREi_CAMERA_GUID &guid1, const FIREi_CAMERA_GUID &guid2);

	static std::string guidToHex(const FIREi_CAMERA_GUID &guid);
	static long long guidToLL(const FIREi_CAMERA_GUID &guid);
	static FIREi_CAMERA_GUID llToGUID(const long long llid);

	static std::string getString(FIREi_CAMERA_HANDLE camHandle, FIREi_OID oidCode);
	static void throwLastError(const std::string &prefix, FIREi_STATUS status);

private:
	static unsigned long nbUnibrainObj_;
	static std::set<long long> usedGUID_;

	FIREi_CAMERA_GUID guid_;
	FIREi_CAMERA_HANDLE handle_;
	FIREi_CAMERA_STARTUP_INFO startupInfo_;
	UBUI_SETTINGS_HANDLE settings_;
	FIREi_ISOCH_ENGINE_HANDLE engine_;

	bool mono_;
	bool doBayer_;

};


class SLCAMIN_DLL_EXPORT slUnibrainCamFactory: public slCamInFactory
{
public:
	virtual ~slUnibrainCamFactory();

protected:
	// To specify your functions's parameters
	void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const;

	// The factory (a static instance) will create an instance of your slCamIn
	slUnibrainCam* createInstance() const;

private:
	// The factory's constructor
	slUnibrainCamFactory();

private:
	static slUnibrainCamFactory factory_;

};


#endif	// SLUNIBRAINCAM_H


