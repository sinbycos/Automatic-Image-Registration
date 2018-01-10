/*!	\file	slIn.h
 *	\brief	Contains an abstract class for image sources
 *
 *	This file contains the definition of the class slIn.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		April 2010
 */

#ifndef SLIN_H
#define SLIN_H


#include "slIO.h"


//!	Base class for all image sources
/*!
 *	This class gives a basic common interface for image sources like slImageIn
 *	and slVideoIn.
 *	With function read(slMat &), it is possible to load one image at a time.
 *
 *	\see		slImageIn, slVideoIn, slOut
 *	\author		Pier-Luc St-Onge
 *	\date		April 2010
 */
class SLIO_DLL_EXPORT slIn
{
public:
	slIn();
	virtual ~slIn();

	virtual slIn& open(const std::string &name) = 0;	//!< Opens a video or a camera connection
	virtual void read(slMat &image) = 0;		//!< Reads the next image from a video or the image file
	virtual void close() = 0;			//!< Closes a video file or a camera connection

	double getFPS() const;				//!< Returns number of frames per second
	int getHeight() const;				//!< Returns image(s) height
	unsigned int getNbImages() const;	//!< Returns number of images
	cv::Size getSize() const;			//!< Returns image(s) size
	int getWidth() const;				//!< Returns image(s) width

protected:
	void resetSpecs();

protected:
	int width_;
	int height_;

	double fps_;
	unsigned int nbImages_;

};


#endif	// SLIN_H


