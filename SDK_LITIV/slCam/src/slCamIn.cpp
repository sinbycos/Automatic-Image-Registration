/*!	\file	slCamIn.cpp
 *	\brief	This file contains the code of slCamIn
 *
 *	\author		Pier-Luc St-Onge
 *	\date		May 2011
 */

#include "stdafx.h"
#include "slCamIn.h"

#include <sstream>


using namespace cv;
using namespace std;
using namespace slAH;


slCamIn::slCamIn(): slIn()
{
	connected_ = false;
	started_ = false;
	resetDescription();
}


slCamIn::~slCamIn()
{
}


void slCamIn::fillAllParamSpecs(slParamSpecMap& paramSpecMap)
{
	slCamInFactory::fillAllParamSpecs(paramSpecMap);
}


bool slCamIn::isConnected()
{
	return connected_;
}


std::string slCamIn::description() const
{
	stringstream sstr;
	string desc2;

	sstr << brand_ << " " << model_ << ", " << width_ << "x" << height_ << ", " << fps_ << " ips";

	desc2 = description2();
	if (!desc2.empty()) sstr << ", " << desc2;

	return sstr.str();
}


std::string slCamIn::description2() const
{
	return "";
}


bool slCamIn::inColors() const
{
	return true;
}


void slCamIn::showParameters()
{
}


void slCamIn::hideParameters()
{
}


/*!
 *	\param delay is the private time to wait (in ms) before triggering the capture of one image.
 *		All slCamIn objects own a slSynchroSem instance which is initiated with this delay.
 *		The delay is only useful for synchronized multi-threaded captures.
 *		This is why the default value is 0 ms.
 *	\see slSynchroSem
 */
slCamIn& slCamIn::start(int delay)
{
	stop();

	// Créer les nouvelles images
	image1ch_.create(height_, width_);
	image3ch_.create(height_, width_);

	startAfter();

	// Créer le thread de capture
	synchroSem_.reset(delay);
	captureH_ = CreateThread(NULL, NULL, startCaptures, this, NULL, NULL);

	started_ = true;

	return *this;
}


void slCamIn::startAfter()
{
	// Rien par défaut
}


DWORD WINAPI slCamIn::startCaptures(void *args)
{
	return ((slCamIn*)args)->loopCaptures();
}


DWORD slCamIn::loopCaptures()
{
	Mat mat;

	synchroSem_.waitForGo();

	while (synchroSem_.loop()) {
		// Capture
		read(mat);

		synchroSem_.done();
		synchroSem_.waitForGo();
	}

	return 0;
}


slSynchroSemPtr slCamIn::synchro()
{
	return &synchroSem_;
}


const cv::Mat& slCamIn::getOutputImage() const
{
	if (inColors()) {
		return image3ch_;
	}
	else {
		return image1ch_;
	}
}


cv::Scalar slCamIn::getPixel(const cv::Point &pos) const
{
	if (inColors()) {
		return image3ch_[pos.y][pos.x];
	}
	else {
		return image1ch_[pos.y][pos.x];
	}
}


void slCamIn::stopBefore()
{
	// Rien par défaut
}


void slCamIn::stop()
{
	if (started_) {
		started_ = false;

		try {
			// Stop capture thread
			synchroSem_.stopLoop();
			WaitForSingleObject(captureH_, INFINITE);
			CloseHandle(captureH_);

			stopBefore();
		}
		catch (...) {
			AfxMessageBox("Problem in slCamIn::stopBefore().");
		}
	}
}


void slCamIn::resetDescription()
{
	brand_ = "Unknown brand";
	model_ = "Unknown model";
	width_ = 0;
	height_ = 0;
	fps_ = 0.0f;
}


///////////////////////////////////////////////////////////////////////////////
//	slCamInFactory
///////////////////////////////////////////////////////////////////////////////


// Initializing factories list
slCamInFactory::factories_t* slCamInFactory::factories_ = NULL;


slCamInFactory::slCamInFactory(const std::string& camType)
: camType_(camType)
{
	if (factories_ == NULL) {
		factories_ = new factories_t;
	}

	(*factories_)[camType_] = this;
}


slCamInFactory::~slCamInFactory()
{
}


void slCamInFactory::fillAllParamSpecs(slParamSpecMap& paramSpecMap)
{
	// Camera
	slParamSpec specCam(ARG_CAMERA, "Camera type (unibrain, etc.)", MANDATORY);
	specCam << slSyntax("CAMERA");

	if (factories_ != NULL) {
		// Add all parameters for all cameras
		for (factories_t::const_iterator factory = factories_->begin();
			factory != factories_->end(); factory++)
		{
			slParamSpecMap camParams;

			factory->second->fillParamSpecs(camParams);
			specCam.setSubParamSpec(slValVect() << factory->first, camParams);
		}
	}

	paramSpecMap << specCam;
}


slCamIn* slCamInFactory::createInstance(const char *camInType)
{
	if (factories_ == NULL) {
		throw slExceptionCamIn("slCamInFactory::createInstance(): no factory available");
	}

	// Find the factory corresponding to camInType
	string camType(camInType);
	factories_t::const_iterator itFactory = factories_->find(camType);

	// If the factory does not exist
	if (itFactory == factories_->end()) {
		throw slExceptionCamIn("Bad camera type : \"" + camType + "\"");
	}

	// Create a new instance of the specified camera
	return itFactory->second->createInstance();
}


slCamIn* slCamInFactory::createInstance(const slParameters& parameters)
{
	// Create a new instance of the specified camera
	slCamIn *camIn = createInstance(parameters.getValue(ARG_CAMERA).c_str());

	return camIn;
}


