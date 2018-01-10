/*!	\file	slVideoOut.h
 *	\brief	Contains a class for saving videos
 *
 *	This file contains the definition of the class slVideoOut.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		June 2010
 */

#ifndef SLVIDEOOUT_H
#define SLVIDEOOUT_H


#include "slOut.h"


//!	Opens a file, saves images and closes the file
/*!
 *	This is the class used to save videos.
 *	Example:
 *	\code
 *	slWindow monitor("Monitor");
 *
 *	slVideoIn videoReader;	
 *	videoReader.open("video1.avi");
 *
 *	slImage3ch inputImage, outputImage;
 *	const Size size(320, 240);
 *
 *	slVideoOut videoWriter;
 *	videoWriter.setFPS(videoReader.getFPS());
 *	videoWriter.setSize(size);
 *	videoWriter.open("output.avi");
 *
 *	for (unsigned i = 0; i < videoReader.getNbImages(); i++) {
 *		videoReader.read(inputImage);
 *		resize(inputImage, outputImage, size);
 *		videoWriter.write(outputImage);
 *
 *		monitor.show(outputImage);
 *		if (waitKey(33) != -1) throw slExceptionIO("Problem in video recording");
 *	}
 *
 *	videoWriter.close();
 *	videoReader.close();
 *	\endcode
 *
 *	\see		slOut, slVideoIn, slImageOut, slImageIn
 *	\author		Pier-Luc St-Onge
 *	\date		June 2010
 */
class SLIO_DLL_EXPORT slVideoOut: public slOut
{
public:
	slVideoOut();			// Constructor
	virtual ~slVideoOut();

	void setCodec(int fourcc = -1);			//!< When FOURCC is -1, a popup list shows all available codecs
	void setFPS(double fps);				//!< To set the frame rate of the video
	void setIsColor(bool isColor = true);	//!< True: color images (default), False: grayscale images
	void setSize(cv::Size size);			//!< To set the size of each frame

	slVideoOut& open(const std::string &name);	//!< Opens the video file
	void write(const slMat &image);				//!< Saves one image
	void close();			//!< Close the video file

private:
	void releaseWriter();

private:
	int fourcc_;
	int width_;
	int height_;
	double fps_;
	bool isColor_;

	cv::VideoWriter *writer_;

};


#endif	// SLVIDEOOUT_H


