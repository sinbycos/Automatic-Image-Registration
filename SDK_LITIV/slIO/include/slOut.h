/*!	\file	slOut.h
 *	\brief	Contains an abstract class for saving images as images or videos
 *
 *	This file contains the definition of the class slOut.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		June 2010
 */

#ifndef SLOUT_H
#define SLOUT_H


#include "slIO.h"


//! Abstract class for saving images
/*!
 *	This class gives a basic common interface for slImageOut and slVideoOut.
 *	With function write(const slMat &), it is possible to save one image at a time.
 *
 *	\see		slImageOut, slVideoOut, slIn
 *	\author		Pier-Luc St-Onge
 *	\date		June 2010
 */
class SLIO_DLL_EXPORT slOut
{
public:
	slOut();
	virtual ~slOut();

	virtual slOut& open(const std::string &name) = 0;	//!< Opens target file
	virtual void write(const slMat &image) = 0;			//!< Saves one image
	virtual void close() = 0;							//!< Close any opened file

};


#endif	// SLOUT_H


