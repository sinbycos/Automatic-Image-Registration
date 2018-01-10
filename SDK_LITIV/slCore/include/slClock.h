/*!	\file	slClock.h
 *	\brief	Contains a class that measures the elapsed time
 *
 *	This file contains the definition of the class slClock.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		April 2011
 */

#ifndef SLCLOCK_H
#define SLCLOCK_H


#include <time.h>

#include "slCore.h"


//!	Like a chronometer, this class measures the elapsed time
/*!
 *	This class works like a chronometer that never stops.
 *	You first need to start() it.  Then, each time you stop() it, the clock will
 *	save the elapsed time since we called start() the last time.
 *	The function elapsed() lets you retrieve the elapsed time.
 *
 *	That is not all.  Since SDK_LITIV is made for video analysis, slClock
 *	could also be aware of a period or an expected FPS (number of frames per second).
 *	In both cases, slClock will work with a period in milliseconds with the
 *	precision of a double data type.
 *	For real time applications that need a clock synchronisation, we just have
 *	to call nextDelay() in order to get the time to wait at the end of each
 *	iteration of the process.  Here is an example:
 *	\code
 *	slClock chronometer;
 *	slVideoIn source;
 *	slMat image;
 *	slWindow outputWindow("Video");
 *
 *	source.open("video1.avi");
 *	chronometer.setFPS(source.getFPS());
 *	chronometer.start();
 *
 *	for (unsigned i = 0; i < source.getNbImages(); i++) {
 *		source.read(image);
 *		if (image.empty()) throw slException("First image is empty");
 *
 *		outputWindow.show(image);
 *		waitKey(chronometer.nextDelay());
 *	}
 *
 *	chronometer.stop();
 *	source.close();
 *	\endcode
 *
 *	\author		Pier-Luc St-Onge
 *	\date		April 2011
 */
class SLCORE_DLL_EXPORT slClock
{
public:
	slClock(double period = 1001.0 / 30000);	//!< Default constructor for 29.97 fps
	~slClock();

	void setFPS(double fps);		//!< Sets the period with the wanted FPS
	void setPeriod(double period);	//!< Sets the period explicitly

	void start();			//!< Stop all, reset nbFrames and save start time
	clock_t nextDelay();	//!< Returns the time to wait (ms)
	void stop();			//!< Save stop time

	clock_t elapsed() const;		//!< Elapsed time (ms)
	unsigned int nbFrames() const;	//!< Number of frames
	double getMeasuredFPS() const;	//!< Effective FPS

private:
	double periodInClocks_;	// In clock ticks (ms)

	clock_t startTime_, stopTime_;
	unsigned int nbFrames_;

};


#endif	// SLCLOCK_H


