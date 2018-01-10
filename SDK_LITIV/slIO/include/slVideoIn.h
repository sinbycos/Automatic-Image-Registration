/*!	\file	slVideoIn.h
 *	\brief	Contains a class for loading videos
 *
 *	This file contains the definition of the class slVideoIn.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		April 2010
 */

#ifndef SLVIDEOIN_H
#define SLVIDEOIN_H


#include "slIn.h"


//!	Class used to load one frame at a time from a video file
/*!
 *	This is the class used to load videos and get access to one frame after the
 *	other.
 *	Example:
 *	\code
 *	slMat image;
 *	slWindow monitor("Monitor");
 *
 *	slVideoIn video;
 *	video.open("video1.avi");
 *
 *	slClock myClock;
 *	myClock.setFPS(video.getFPS());
 *	myClock.start();
 *
 *	for (unsigned i = 0; i < video.getNbImages(); i++) {
 *		video.read(image);
 *		if (image.empty()) throw slExceptionIO("First image is empty");
 *
 *		monitor.show(image);
 *		if (waitKey(myClock.nextDelay()) != -1) throw slExceptionIO("Problem in video playback");
 *	}
 *
 *	myClock.stop();
 *	video.close();
 *	\endcode
 *
 *	\see		slIn, slVideoOut, slImageIn, slImageOut
 *	\author		Pier-Luc St-Onge
 *	\date		April 2010
 */
class SLIO_DLL_EXPORT slVideoIn: public slIn
{
public:
	slVideoIn();			//!< Constructor
	virtual ~slVideoIn();

	slVideoIn& open(const std::string &name);	//!< Opens the video file
	void read(slMat &image);					//!< Reads next frame from video file
	void close();			//!< Closes video file

private:
	void releaseCapture();

private:
	cv::VideoCapture *capture_;

};


#endif	// SLVIDEOIN_H


