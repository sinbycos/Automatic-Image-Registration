/*!	\file	slCamIn.h
 *	\brief	This file contains the abstract class for all camera classes.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		May 2011
 */

#ifndef SLCAMIN_H
#define SLCAMIN_H


#ifndef SLCAMIN_DLL_EXPORT
	#ifdef WIN32
		#define SLCAMIN_DLL_EXPORT __declspec(dllexport)
	#else
		#define SLCAMIN_DLL_EXPORT
	#endif
#endif


#include <slArgHandler.h>
#include <slCameraSynchro.h>
#include <slIn.h>


#define ARG_CAMERA "-c"


//!	This is the base class for all cameras
/*!
 *	Classes slIpelaCam (Sony Ipela PTZ or not PTZ), slUnibrainCam (Sony IIDC)
 *	share the same interface which is defined in the current class.
 *
 *	\section constructors Constructors and Factories
 *	The current class is abstract, so we cannot create a slCamIn directly.
 *	As seen in derived classes, it is possible to create a camera object directly:
 *	\code
 *	slIpelaCam ipelaCam;
 *	slUnibrainCam unibrainCam;
 *	// Configuration...
 *	\endcode
 *
 *	If the type of camera is only known at runtime, then
 *	we could use the factory:
 *	\code
 *	slCamIn *cam1 = slCamInFactory::createInstance("ipela");
 *	slCamIn *cam2 = slCamInFactory::createInstance("unibrain");
 *
 *	delete cam1;
 *	delete cam2;
 *	\endcode
 *
 *	\section slCamIn_compute How it works
 *	As simple as:
 *	\code
 *	cam->open();
 *	// OR
 *	cam->open("ip_address");	// Connects to camera
 *
 *	cam->start();				// Starts capture
 *
 *	Mat image;
 *
 *	loop() {
 *		cam->read(image);		// Captures one image
 *		// Do something with the image
 *	}
 *
 *	cam->stop();				// Stops capture
 *	cam->close();				// Disconnect from camera
 *	\endcode
 *
 *	\section slCamIn_synchro How to synchronize the cameras with threads
 *	\code
 *	cam1->open();
 *	cam2->open();
 *
 *	double fps1 = cam1->getFPS();
 *	double fps2 = cam2->getFPS();
 *
 *	slClock fpsClock;
 *	fpsClock.setFPS(fps1 < fps2 ? fps1 : fps2);
 *
 *	slCameraSynchro synchro;
 *	Mat image1, image2;
 *	int delay1 = 10;	// ms
 *	int delay2 = 0;		// ms
 *
 *	synchro.insert(cam1->start(delay1).synchro());
 *	synchro.insert(cam2->start(delay2).synchro());
 *	fpsClock.start();
 *
 *	while (capture) {
 *		// This will trigger both captures at the same time according to the individual delay
 *		// This function returns when both captures are done
 *		synchro.goAndWait();
 *
 *		image1 = cam1->getOutputImage();
 *		image2 = cam2->getOutputImage();
 *
 *		waitKey(fpsClock.nextDelay());
 *	}
 *
 *	cam1->stop();	cam1->close();
 *	cam2->stop();	cam2->close();
 *	\endcode
 *
 *	\see		slIn, slIpelaCam, slUnibrainCam, slCameraSynchro
 *	\author		Pier-Luc St-Onge
 *	\date		May 2011
 */
class SLCAMIN_DLL_EXPORT slCamIn: public slIn
{
public:
	slCamIn();
	virtual ~slCamIn();

	static void fillAllParamSpecs(slAH::slParamSpecMap& paramSpecMap);	//!< To get the syntax of all parameters of all cameras

	virtual slCamIn& open(const std::string &name = "") = 0;	//!< Connects to the camera
	bool isConnected();											//!< Returns true if connected

	std::string description() const;							//!< Main description: brand, model, size, fps
	virtual std::string description2() const;					//!< Other elements of description

	virtual bool inColors() const;					//!< True if result is 3 channels per pixel

	virtual bool hasParameters() const = 0;			//!< True if there is a GUI to edit camera parameters
	virtual bool hasDynamicParameters() const = 0;	//!< True if parameters can be modified while camera is capturing
	virtual void showParameters();					//!< Show parameters window
	virtual void hideParameters();					//!< Hide parameters window

	slCamIn& start(int delay = 0);	//!< Starts capture and sets private delay of synchronisation
	slSynchroSemPtr synchro();		//!< Returns synchronisation dual-semaphore object
	void stop();					//!< Stops capture

	virtual const cv::Mat& getOutputImage() const;				//!< The last captured image
	virtual cv::Scalar getPixel(const cv::Point &pos) const;	//!< Get pixel value

protected:
	void resetDescription();

	virtual void startAfter();
	virtual void stopBefore();

protected:
	bool connected_;
	bool started_;

	std::string brand_;
	std::string model_;

	slImage1ch image1ch_;
	slImage3ch image3ch_;

private:
	static DWORD WINAPI startCaptures(void *args);
	DWORD loopCaptures();			// Internal loop for multi-threaded capturing

private:
	slSynchroSem synchroSem_;
	HANDLE captureH_;

};


class SLCAMIN_DLL_EXPORT slCamInFactory
{
public:
	virtual ~slCamInFactory();

	static void fillAllParamSpecs(slAH::slParamSpecMap& paramSpecMap);

	static slCamIn* createInstance(const char *camInType);
	static slCamIn* createInstance(const slAH::slParameters& parameters);

protected:
	// The factory's constructor
	slCamInFactory(const std::string& camType);

	// To specify your functions's parameters
	virtual void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const = 0;

	// The factory (a static instance) will create an instance of your BgSubtractor
	virtual slCamIn* createInstance() const = 0;

protected:
	std::string camType_;

private: // Types
	typedef std::map<std::string, slCamInFactory*> factories_t;	// List of static factories

private:
	// Because of this, it is not possible to create
	// your own algorithm outside the current project
	static factories_t *factories_;

};


#endif	// SLCAMIN_H


