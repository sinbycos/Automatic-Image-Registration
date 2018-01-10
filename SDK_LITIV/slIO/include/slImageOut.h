/*!	\file	slImageOut.h
 *	\brief	Contains a class for saving images
 *
 *	This file contains the definition of the class slImageOut.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		August 2010
 */

#ifndef SLIMAGEOUT_H
#define SLIMAGEOUT_H


#include "slOut.h"


//!	This is the class used to save images
/*!
 *	It is possible to save an image manually:
 *	\code
 *	slImageOut("output.png").write(slImage3ch(cvSize(320, 240)));
 *	\endcode
 *
 *	This class can also be used to save sequences of images.  The filename
 *	format follows the rules of function \c printf().  An index (32 bits
 *	integer) is used to create a different file for each image.
 *	\code
 *	slImage3ch image(cvSize(320, 240));
 *	slImageOut imageOut("output%04d.png", 1);
 *
 *	imageOut.write(image); // creates output0001.png
 *	image = slRGB(255, 0, 0);
 *	imageOut.write(image); // creates output0002.png
 *	\endcode
 *
 *	\see		slOut, slImageIn, slVideoOut, slVideoIn
 *	\author		Pier-Luc St-Onge
 *	\date		August 2010
 */
class SLIO_DLL_EXPORT slImageOut: public slOut
{
public:
	slImageOut(const std::string &filename_format = "", int index = 0);	//!< Constructor, sets filename format and first index
	virtual ~slImageOut();

	slImageOut& open(const std::string &filename_format);	//!< Sets the filename format
	void write(const slMat &image);		//!< Saves one image
	void close();						//!< Clears the filename format and resets index to 0

	slImageOut& setFormat(const std::string &filename_format);	//!< Sets the filename format
	slImageOut& setIndex(int index);							//!< Sets the first index

private:
	std::string format_;
	int index_;

};


#endif	// SLIMAGEOUT_H


