/*!	\file	slImageIn.h
 *	\brief	Contains a class that loads an image from a file
 *
 *	This file contains the definition of the class slImageIn.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		August 2010
 */

#ifndef SLIMAGEIN_H
#define SLIMAGEIN_H


#include "slIn.h"


//!	Class used to load a slImage3ch (or Mat) object from a file
/*!
 *	Example:
 *	\code
 *	slImage3ch image;
 *	slImageIn().open("test.png").read(image);
 *	\endcode
 *
 *	It is also possible to read a sequence of images:
 *	\code
 *	slImageIn imageIn("output%04d.png", 1);
 *
 *	imageIn.read(image);	// loads output0001.png
 *	imageIn.read(image);	// loads output0002.png
 *	\endcode
 *
 *	\see		slIn, slImageOut, slVideoIn, slVideoOut
 *	\author		Pier-Luc St-Onge
 *	\date		August 2010
 */
class SLIO_DLL_EXPORT slImageIn: public slIn
{
public:
	slImageIn(const std::string &filename_format = "", int index = 0);	//!< Constructor, sets filename format and first index
	virtual ~slImageIn();

	slImageIn& open(const std::string &filename_format);	//!< Sets a new filename format and loads image attributes
	void read(slMat &image);			//!< Loads one image
	void close();						//!< Clears the filename format and resets index to 0

	slImageIn& setFormat(const std::string &filename_format);	//!< Sets the filename format
	slImageIn& setIndex(int index);								//!< Sets the first index

private:
	std::string format_;
	int index_;

};


#endif	// SLIMAGEIN_H


