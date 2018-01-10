/*!	\file	slIpelaCam.cpp
 *	\brief	This file contains the implementation of class slIpelaCam (for PTZ camera).
 *
 *	\author		Pier-Luc St-Onge
 *	\date		October 2011
 */

#include "stdafx.h"
#include "slIpelaCam.h"

#include <iostream>


using namespace cv;
using namespace slAH;
using namespace std;


////////////////
// slIpelaWeb //
////////////////


slIpelaWeb::slIpelaWeb():
host_(""), username_(""), password_(""),
internetHandle_(NULL), cameraHandle_(NULL), requestHandle_(NULL)
{
}


slIpelaWeb::~slIpelaWeb()
{
	close();
}


void slIpelaWeb::setCredentials(const std::string &username, const std::string &password)
{
	username_ = username;
	password_ = password;
}


void slIpelaWeb::open(const std::string &host)
{
	// New host
	host_ = host;

	// open internet connection
	internetHandle_ = InternetOpen("slIpelaWeb", INTERNET_OPEN_TYPE_PRECONFIG, NULL, 
		INTERNET_INVALID_PORT_NUMBER, 0);

	if (internetHandle_ == NULL) {
		throw slExceptionCamIn("slIpelaWeb::open(): InternetOpen() failed");
	}
	
	// connect to CameraIP server
	cameraHandle_ = InternetConnect(internetHandle_, host_.c_str(), 
		INTERNET_INVALID_PORT_NUMBER, "", "", INTERNET_SERVICE_HTTP, 0,0);

	if (cameraHandle_ == NULL) {
		throw slExceptionCamIn("slIpelaWeb::open(): InternetConnect() failed");
	}
}


BOOL slIpelaWeb::read(LPVOID buffer, DWORD nbBytesMax, LPDWORD nbBytesRead)
{
	return InternetReadFile(requestHandle_, buffer, nbBytesMax, nbBytesRead);
}


void slIpelaWeb::close()
{
	if (requestHandle_ != NULL) {
		InternetCloseHandle(requestHandle_);
		requestHandle_ = NULL;
	}

	if (cameraHandle_ != NULL) {
		InternetCloseHandle(cameraHandle_);
		cameraHandle_ = NULL;
	}

	if (internetHandle_ != NULL) {
		InternetCloseHandle(internetHandle_);
		internetHandle_ = NULL;
	}
}


void slIpelaWeb::sendInfoRequest(const char *request)
{
	const char *httpVersion = "HTTP/1.0";

	if (requestHandle_ != NULL) {
		InternetCloseHandle(requestHandle_);
	}

	// request the script file from the camera server.
	requestHandle_ = HttpOpenRequest(cameraHandle_, "GET", request,
		httpVersion, NULL, NULL, INTERNET_FLAG_KEEP_CONNECTION, 0);

	if (requestHandle_ == NULL) {
		throw slExceptionCamIn("slIpelaWeb::sendInfoRequest(): HttpOpenRequest() failed");
	}

	// Set option header
	char *optionHeader = "Accept: text/*, image/*\r\nUser-Agent:  slIpelaWebClient/1.0\r\n";

	if (!HttpAddRequestHeaders(requestHandle_, optionHeader, -1, HTTP_ADDREQ_FLAG_ADD)) {
		cerr << "The headers have not been replaced !" << endl;
	}

	// Send and resend request if needed
	DWORD status = 0, statusSize = sizeof(status);
	bool resend = false;	// Once, by default

	do {
		// send the request
		HttpSendRequest(requestHandle_, NULL, 0, NULL, 0);
	
		// get the HTTP status code
		if (!HttpQueryInfo(requestHandle_, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &status, &statusSize, NULL)) {
			if ((GetLastError() == ERROR_HTTP_HEADER_NOT_FOUND) || (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
				cerr << "The header status code isn't available !" << endl;
			}
		}

		switch (status)
		{
			case HTTP_STATUS_DENIED:     // Server Authentication Required.			
				InternetSetOption(requestHandle_, INTERNET_OPTION_USERNAME, (char*)username_.c_str(), (DWORD)username_.length() + 1);
				InternetSetOption(requestHandle_, INTERNET_OPTION_PASSWORD, (char*)password_.c_str(), (DWORD)password_.length() + 1);
				break;
			case HTTP_STATUS_OK:  // The request has succeeded
#if 0
				cout << "The request has succeeded !" << endl;
#endif
				break;
			case HTTP_STATUS_REQUEST_TIMEOUT:  //server timed out
				cout << "The server timed out waiting for the request!" << endl;
				break;
			default: 
				cout << "The host was not reached!" << endl;
				break;
		}

		// Try to send the request one more time
		resend = (status == HTTP_STATUS_DENIED && !resend);
	} while (resend);
}


std::string slIpelaWeb::recvInfoRequest()
{
	if (requestHandle_ == NULL) {
		throw slExceptionCamIn("slIpelaWeb::recvInfoRequest(): request handle is NULL");
	}

	// Coping in 100 bytes
	unsigned long bytesRead;
	char info[101];

	if (!InternetReadFile(requestHandle_, info, 100, &bytesRead)) {
		throw slExceptionCamIn("slIpelaWeb::recvInfoRequest(): InternetReadFile() failed");
	}

	if (bytesRead == 0) {
		throw slExceptionCamIn("slIpelaWeb::recvInfoRequest(): no byte received, request failed");
	}

	info[bytesRead] = '\0';

	return string(info);
}


bool slIpelaWeb::sendCommand(const char *request, const char *vars)
{
	const char *httpVersion = "HTTP/1.0";
	const char *contentType = "Content-Type: application/x-www-form-urlencoded";

	if (requestHandle_ != NULL) {
		InternetCloseHandle(requestHandle_);
	}
	
	// post the data to the CameraIP.
	requestHandle_ = HttpOpenRequest(cameraHandle_, "POST", request,
		httpVersion, NULL, NULL, INTERNET_FLAG_KEEP_CONNECTION, 0);

	if (requestHandle_ == NULL) {
		throw slExceptionCamIn("slIpelaWeb::sendCommand(): HttpOpenRequest() failed");
	}

	// Send and resend request if needed
	DWORD status = 0, statusSize = sizeof(status);
	bool resend = false;	// Once, by default

	char locVars[250];
	strcpy_s(locVars, 250, vars);

	do {
		// send the request
		HttpSendRequest(requestHandle_, contentType, -1, locVars, (DWORD)strlen(locVars));

		// get the HTTP status code
		if (!HttpQueryInfo(requestHandle_, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &status, &statusSize, NULL)) {
			if ((GetLastError() == ERROR_HTTP_HEADER_NOT_FOUND) || (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
				cerr << "The header status code isn't available !" << endl;
			}
		}

		switch (status)
		{
			case HTTP_STATUS_DENIED:     // Server Authentication Required.
				InternetSetOption(requestHandle_, INTERNET_OPTION_USERNAME, (char*)username_.c_str(), (DWORD)username_.length() + 1);
				InternetSetOption(requestHandle_, INTERNET_OPTION_PASSWORD, (char*)password_.c_str(), (DWORD)password_.length() + 1);
				break;
			case HTTP_STATUS_NO_CONTENT:
#if 0
				cerr << "The camera received the POST values and has no content to output." << endl;
#endif
				break;
			case HTTP_STATUS_OK:  // The request has succeeded
#ifdef _DEBUG
				cout << "The request has succeeded !" << endl;
#endif
				break;
			case HTTP_STATUS_REQUEST_TIMEOUT:  //server timed out
				cerr << "The server timed out waiting for the request!" << endl;
				break;
			default: 
				cerr << "The host was not reached!" << endl;
				break;
		}

		// Try to send the request one more time
		resend = (status == HTTP_STATUS_DENIED && !resend);
	} while (resend);

	return (status == HTTP_STATUS_OK || status == HTTP_STATUS_NO_CONTENT);
}


////////////////////////////
// slIpelaCam::ControlSem //
////////////////////////////


slIpelaCam::ControlSem::ControlSem()
{
	go_ = CreateSemaphore(NULL, 0, 1, NULL);
	stop_ = false;
}


void slIpelaCam::ControlSem::reset()
{
	CloseHandle(go_);
	go_ = CreateSemaphore(NULL, 0, 1, NULL);
	stop_ = false;
}


slIpelaCam::ControlSem::~ControlSem()
{
	CloseHandle(go_);
}


void slIpelaCam::ControlSem::go()
{
	ReleaseSemaphore(go_, 1, NULL);
}


void slIpelaCam::ControlSem::waitForGo()
{
	WaitForSingleObject(go_, INFINITE);
}


bool slIpelaCam::ControlSem::loop() const
{
	return !stop_;
}


void slIpelaCam::ControlSem::stopLoop()
{
	stop_ = true;
	go();
}


////////////////
// slIpelaCam //
////////////////


slIpelaCam::slIpelaCam(): slCamIn()
{
	controlCmdsMutex_ = CreateMutex(NULL, false, NULL);
}


slIpelaCam::~slIpelaCam()
{
	stop();
	close();

	if (controlCmdsMutex_ != NULL) {
		CloseHandle(controlCmdsMutex_);
		controlCmdsMutex_ = NULL;
	}
}


void slIpelaCam::setCredentials(const std::string &username, const std::string &password)
{
	if (!connected_) {
		capture_.setCredentials(username, password);
		control_.setCredentials(username, password);
	}
}


slIpelaCam& slIpelaCam::open(const std::string &host)
{
	// Close all
	close();

	// Open all
	capture_.open(host);
	control_.open(host);

	// Verify connection and set all parameters
	getSetSpecifications();

	// Apply all camera parameters
	if (!setMode(IPELA_JPEG, width_)) {
		throw slExceptionCamIn("slIpelaCam::open(): bad mode or bad size");
	}

	// Fin de la connexion réussie
	connected_ = true;

	return *this;
}


void slIpelaCam::getSetSpecifications()
{
	brand_ = "Sony Ipela";

	// Get model of the camera
	capture_.sendInfoRequest("/command/inquiry.cgi?inq=system");
	string temp = capture_.recvInfoRequest();

	if (temp != "0") {
		size_t position = temp.find("ModelName=");

		if (position != string::npos && temp.length() >= (position + 19)) {
			position += 10;
			model_ = temp.substr(position, 9);
		}
		else {
			throw slExceptionCamIn("slIpelaCam::getSetSpecifications(): cannot parse ModelName");
		}
	}
	else {
		throw slExceptionCamIn("slIpelaCam::getSetSpecifications(): cannot get model");
	}

	// Obtenir la taille des images et le framerate
	width_ = 320;
	height_ = 240;
	fps_ = 30;
}


bool slIpelaCam::hasParameters() const
{
	return false;
}


bool slIpelaCam::hasDynamicParameters() const
{
	return false;
}


void slIpelaCam::startAfter()
{
	// Démarrer le thread pour la portion PTZ
	controlSem_.reset();
	controlH_ = CreateThread(NULL, NULL, startControls, this, NULL, NULL);
}


DWORD WINAPI slIpelaCam::startControls(void *args)
{
	return ((slIpelaCam*)args)->loopControls();
}


DWORD slIpelaCam::loopControls()
{
	controlSem_.waitForGo();

	while (controlSem_.loop()) {
		// Control command
		WaitForSingleObject(controlCmdsMutex_, INFINITE);
		ControlCmd command = controlCmds_.front();
		controlCmds_.pop_front();
		ReleaseMutex(controlCmdsMutex_);

		// Execute command
		control_.sendCommand(command.request.c_str(), command.vars.c_str());

		controlSem_.waitForGo();
	}

	return 0;
}


bool slIpelaCam::sendControlCommand(const char *request, const char *vars)
{
	bool sentCommand = false;

	WaitForSingleObject(controlCmdsMutex_, INFINITE);

	if (controlCmds_.empty()) {
		// Add a control task
		controlCmds_.push_back(ControlCmd(request, vars));

		// Unlock control loop
		controlSem_.go();
		sentCommand = true;
	}

	ReleaseMutex(controlCmdsMutex_);

	return sentCommand;
}


void slIpelaCam::read(slMat &image)
{
	try {
		capture_.sendInfoRequest("/oneshotimage");

		// Copying in 100000 bytes 
		Mat jpegBytes(100000, 1, CV_8UC1);
		unsigned long bytesRead;

		if (!capture_.read(jpegBytes.data, jpegBytes.rows, &bytesRead)) {
			throw slExceptionCamIn("slIpelaCam::read(): InternetReadFile failed");
		}

		image3ch_ = imdecode(jpegBytes(Range(0, bytesRead), Range::all()), 1);
		image = image3ch_;
	}
	catch (...) {
		image = Mat();
	}
}


void slIpelaCam::stopBefore()
{
	// Arrêter le thread pour la portion PTZ
	controlSem_.stopLoop();
	WaitForSingleObject(controlH_, INFINITE);
	CloseHandle(controlH_);
}


void slIpelaCam::close()
{
	// La caméra ne sera plus utilisée
	connected_ = false;

	// Reset de la description
	resetDescription();

	// Tout fermer
	capture_.close();
	control_.close();
}


/*!
 *	\param wBal 0 = normal auto, 1 = indoor, 2 = outdoor,
 *		3 = one push WB, 4 = auto tracing white, 5 = manual
 *	\return true if successful
 */
bool slIpelaCam::whiteBalance(char wBal)
{
	char vars[50];
	
	if (wBal < '0' || wBal > '5') return false;
	
	if (model_ == "SNC-RZ25N" || model_ == "SNC-RZ50N") {
		sprintf_s(vars, "visca=810104350%cFF", wBal);
		return capture_.sendCommand("/command/visca-ptzf.cgi", vars);
	}
	else if (model_ == "SNC-CS11&" || model_ == "SNC-CS50N") {
		sprintf_s(vars, "camera=810104350%cFF", wBal);
		return capture_.sendCommand("/command/camera.cgi", vars);
	}
	else {
		return false;
	}
}


/*!
 *	Also called "absolute zoom".
 *
 *	\param zoom zoom factor (position) 0 to 16384
 *	\return true if successful
 */
bool slIpelaCam::absZoom(unsigned int zoom)
{
	char vars[50], bidon[5];

	if (zoom > 16384) return false;

	strcpy_s(vars, 15, "AbsoluteZoom=");
	sprintf_s(bidon, "%x", zoom);
	strcat_s(vars, bidon);  // append var value to var name

	return sendControlCommand("/command/ptzf.cgi", vars);
}


/*!
 *	Absolute Zoom for a set multiplier zoom (1x,2x,3x,...,18x).
 *
 *	\param zoomMultiplier zoom multiplier (1,2,3,4,...,18) converted to zoom position [0,16384]
 *	\return true if successful
 *	\see absZoom()
 */
bool slIpelaCam::absMultiZoom(unsigned int zoomMultiplier)
{
	unsigned int zoom = 65536;

	switch (zoomMultiplier)
	{
	case 1:		zoom = 0;		break;
	case 2:		zoom = 5638;	break;
	case 3:		zoom = 8529;	break;
	case 4:		zoom = 10336;	break;
	case 5:		zoom = 11445;	break;
	case 6:		zoom = 12384;	break;
	case 7:		zoom = 13011;	break;
	case 8:		zoom = 13637;	break;
	case 9:		zoom = 14119;	break;
	case 10:	zoom = 14505;	break;
	case 11:	zoom = 14914;	break;
	case 12:	zoom = 15179;	break;
	case 13:	zoom = 15493;	break;
	case 14:	zoom = 15733;	break;
	case 15:	zoom = 15950;	break;
	case 16:	zoom = 16119;	break;
	case 17:	zoom = 16288;	break;
	case 18:	zoom = 16384;	break;
	default:					break;
	}

	return absZoom(zoom);
}


/*!
 *	Move camera to the specified relative position from the center of image
 *	and/or zoom to a specified rectangle size.
 *	Some other functions are more convenient.
 *
 *	\param relativePos is the relative position to the center of the image (width and
 *		height divided by 2) which corresponds to the center of field of view.
 *		In other words, if relativePos==Point(0,0), the camera stays at the same orientation.
 *	\param zarea is the zooming area.
 *		If zarea is greater than the image size, the camera will zoom out.
 *		If zarea is smaller than the image size, the camera will zoom in.
 *		Otherwise, when zarea is equal to the image size, no zooming is done.
 *	\return true if successful
 */
bool slIpelaCam::areaZoom(cv::Point relativePos, cv::Size zarea)
{
	char vars [50];

	if (relativePos.x < -320 || relativePos.x > 320) return false;
	if (relativePos.y < -240 || relativePos.y > 240) return false;

	if (zarea.width < 0 || zarea.width > 640) return false;
	if (zarea.height < 0 || zarea.height > 480) return false;

	sprintf_s(vars, "AreaZoom=%d,%d,%d,%d", relativePos.x, relativePos.y, zarea.width, zarea.height);

	return sendControlCommand("/command/ptzf.cgi", vars);
}


/*!
 *	Zoom according to a provided rectangle.
 *	The center of the rectangle will become the center of field of view.
 *
 *	\param rect Bounding box in image coordinates (top-left origin).
 *	\return true if successful
 */
bool slIpelaCam::areaZoom(cv::Rect rect)
{
	rect.x += (rect.width - width_) / 2;
	rect.y += (rect.height - height_) / 2;

	return areaZoom(rect.tl(), rect.size());
}


/*!
 *	Zoom according to two provided points.
 *	The two points are automatically converted to a bounding box which includes both points:
 *	\code
 *	Rect rect(pt1, pt2);
 *
 *	rect.width++;
 *	rect.height++;
 *	\endcode
 *
 *	\param pt1 First point in image coordinates (top-left origin)
 *	\param pt2 Second point in image coordinates (top-left origin)
 *	\return true if successful
 */
bool slIpelaCam::areaZoom(cv::Point pt1, cv::Point pt2)
{
	Rect rect(pt1, pt2);

	rect.width++;
	rect.height++;

	return areaZoom(rect);
}


/*!
 *	Simply recenter the field of view according to the selected pixel.
 *
 *	\param pos Center point in image coordinates (top-left origin)
 *	\return true if successful
 */
bool slIpelaCam::recenter(cv::Point pos)
{
	pos.x -= width_ / 2;
	pos.y -= height_ / 2;

	return areaZoom(pos, getSize());
}


/*!
 *	Zoom out by a factor of sqrt(2).  It will also recenter according to the center point.
 *
 *	\param pos Center point in image coordinates (top-left origin)
 *	\return true if successful
 */
bool slIpelaCam::zoomOut(cv::Point pos)
{
	pos.x -= width_ / 2;
	pos.y -= height_ / 2;

	return areaZoom(pos, Size((92682 * width_) >> 16, (92682 * height_) >> 16));
}


bool slIpelaCam::setMode(IpelaMode mode, int width)
{
	char vars[200];

	if (mode == IPELA_JPEG && (width == 320 || width == 384 || width == 640)) {
		if (model_ == "SNC-RZ25N") {
			sprintf_s(vars, "ImageCodec=jpeg&ImageSize=%d,0&JpFrameRate=30&"
				"JpQuality=5&JpBandwidth=4.0", width);
		}
		else if (model_ == "SNC-RZ50N") {
			sprintf_s(vars, "ImageCodec=jpeg&JpImageSize=%d,0&JpFrameRate=30&"
				"JpQuality=%d&JpBandwidth=4.0&JpAreaSelect=off", width, (width == 320 ? 10 : 8));
		}
		else if (model_ == "SNC-CS11&") {	// a modifier 
			sprintf_s(vars, "ImageCodec=jpeg&ImageSize=%d,1&JpFrameRate=30&"
				"JpQuality=10&JpBandwidth=4.0&JpAreaSelect=off", width);
		}
		else if (model_ == "SNC-CS50N") {	// a modifier
			sprintf_s(vars, "ImageCodec=jpeg&JpImageSize=%d,1&JpFrameRate=30&"
				"JpQuality=10&JpBandwidth=4.0&JpAreaSelect=off", width);
		}
		else {
			return false;
		}
	}
	else if (mode == IPELA_MPEG && (width == 320 || width == 640)) {
		if (model_ == "SNC-RZ25N" || model_ == "SNC-RZ50N") {
			sprintf_s(vars, "Color=color&ImageCodec=mpeg4&M4ImageSize=%d,0&M4FrameRate=30&"
				"M4BitRate=2048&M4IFrameInterval=1&M4AutoRateCtrl=off", width);
		}
		else if (model_ == "SNC-CS11&") {	// a modifier
			sprintf_s(vars, "Color=color&ImageCodec=mpeg4&M4ImageSize=%d,1&M4FrameRate=30&"
				"M4BitRate=2048&M4IFrameInterval=1&M4AutoRateCtrl=off", width);
		}
		else if (model_ == "SNC-CS50N") {	// a modifier
			sprintf_s(vars, "Color=color&ImageCodec=mpeg4&M4ImageSize=%d,1&M4FrameRate=30&"
				"M4BitRate=%d&M4IFrameInterval=1&M4AutoRateCtrl=off", width, (width == 320 ? 1056 : 2048));
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}

	return capture_.sendCommand("/command/camera.cgi", vars);
}


///////////////////////////////////////////////////////////////////////////////
//	slIpelaCamFactory
///////////////////////////////////////////////////////////////////////////////


slIpelaCamFactory slIpelaCamFactory::factory_;


slIpelaCamFactory::slIpelaCamFactory()
: slCamInFactory("ipela")
{
}


slIpelaCamFactory::~slIpelaCamFactory()
{
}


void slIpelaCamFactory::fillParamSpecs(slParamSpecMap& paramSpecMap) const
{
}


slIpelaCam* slIpelaCamFactory::createInstance() const
{
	return new slIpelaCam();
}


