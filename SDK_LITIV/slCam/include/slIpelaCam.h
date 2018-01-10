/*!	\file	slIpelaCam.h
 *	\brief	This file contains the classes slIpelaCam (for PTZ camera)
 *		and slIpelaWeb (for PTZ capture and control)
 *
 *	\author		Pier-Luc St-Onge
 *	\date		October 2011
 */

#ifndef SLIPELACAM_H
#define SLIPELACAM_H


#include "slCamIn.h"

#include <deque>
#include <string>
#include <wininet.h>


enum IpelaMode {IPELA_JPEG, IPELA_MPEG};


//!	This class implements the connexion to the IP camera
/*!
 *	It contains three Internet handles: internet, camera and request.
 *	It also contains the host name and the user name and password in order to
 *	authenticate the connexion to the camera.
 *
 *	\see		slIpelaCam, slCamIn
 *	\author		Pier-Luc St-Onge
 *	\date		October 2011
 */
class SLCAMIN_DLL_EXPORT slIpelaWeb
{
public:
	slIpelaWeb();	//!< Constructor
	~slIpelaWeb();

	void setCredentials(const std::string &username, const std::string &password);	//!< Sets username and password

	void open(const std::string &host);	//!< Opens a connection on the IpelaCamera
	BOOL read(LPVOID buffer, DWORD nbBytesMax, LPDWORD nbBytesRead);	//!< Reads upto nbBytesMax bytes
	void close();						//!< Closes the connection
	
	void sendInfoRequest(const char *request);	//!< Sends a request for information
	std::string recvInfoRequest();				//!< Receives the information from a prevously sent request
	bool sendCommand(const char *request, const char *vars);	//!< Sends a general command to camera

private:
	std::string host_;
	std::string username_;
	std::string password_;

	HINTERNET internetHandle_;
	HINTERNET cameraHandle_;
	HINTERNET requestHandle_;

};


//!	This class can connect and control Sony Ipela cameras
/*!
 *	This is the class to use for Ipela PTZ cameras.
 *
 *	This class manages two slIpelaWeb connections to the same camera: one for capturing
 *	images, and one to send control commands in order to use the Pan/Tilt/Zoom
 *	functionnalities of the PTZ cameras.
 *	The control commands are sent with a separate thread, which means the capturing
 *	process remains fluid.
 *
 *	\see		slCamIn, slIpelaWeb
 *	\author		Pier-Luc St-Onge
 *	\date		October 2011
 */
class SLCAMIN_DLL_EXPORT slIpelaCam: public slCamIn
{
public:
	slIpelaCam();			//!< Constructor
	virtual ~slIpelaCam();

	void setCredentials(const std::string &username, const std::string &password);	//!< Sets username and password before connections are opened

	slIpelaCam& open(const std::string &host);	//!< Open connection on IP camera
	void close();								//!< Close camera connexion

	bool hasParameters() const;			//!< Returns false
	bool hasDynamicParameters() const;	//!< Returns false

	void read(slMat &image);			//!< Returns a 3-channels image

	bool whiteBalance(char wBal);		//!< White balance

public: // PTZ
	bool absZoom(unsigned int zoom);						//!< Zoom from 0 to 16384 (low level parameter)
	bool absMultiZoom(unsigned int zoomMultiplier);			//!< Zoom from 1x to 18x (high level zoom factor)

	bool areaZoom(cv::Point relativePos, cv::Size zarea);	//!< Low level zooming function
	bool areaZoom(cv::Rect rect);							//!< Convenient zooming function
	bool areaZoom(cv::Point pt1, cv::Point pt2);			//!< Convenient zooming function
	bool recenter(cv::Point pos);							//!< Convenient centering function
	bool zoomOut(cv::Point pos);							//!< Simple zoom out function

	bool sendControlCommand(const char *request, const char *vars);	//!< Generic send control function

protected:
	void startAfter();
	void stopBefore();

private:
	void getSetSpecifications();

	bool setMode(IpelaMode mode, int width);

private:
	struct ControlCmd
	{
		ControlCmd(const char *req, const char *var): request(req), vars(var) {}

		std::string request, vars;
	};

	class SLCAMIN_DLL_EXPORT ControlSem
	{
	public:
		ControlSem();
		~ControlSem();

		void reset();

		void go();
		void waitForGo();

		bool loop() const;
		void stopLoop();

	protected:
		ControlSem operator=(const ControlSem &ss);

	private:
		HANDLE go_;
		bool stop_;
	};

	static DWORD WINAPI startControls(void *args);
	DWORD loopControls();

private:
	slIpelaWeb capture_;	// Capture connection
	slIpelaWeb control_;	// Control connection

	HANDLE controlCmdsMutex_;				// Only one thread has access to controlCmds_
	std::deque<ControlCmd> controlCmds_;	// A deque of control commands (fifo, limit of 1 item)

	ControlSem controlSem_;	// Semaphore used in the thread that sends control commands
	HANDLE controlH_;		// Handle for the thread that sends control commands

};


class SLCAMIN_DLL_EXPORT slIpelaCamFactory: public slCamInFactory
{
public:
	virtual ~slIpelaCamFactory();

protected:
	// To specify your functions's parameters
	void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const;

	// The factory (a static instance) will create an instance of your slCamIn
	slIpelaCam* createInstance() const;

private:
	// The factory's constructor
	slIpelaCamFactory();

private:
	static slIpelaCamFactory factory_;

};


#endif	// SLIPELACAM_H


