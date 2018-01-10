/*!	\file	slRectPixels.h
 *	\brief	Rectangle used in rectangle background subtractor
 *
 *	\author		Michael Sills Lavoie, Pier-Luc St-Onge
 *	\date		23.05.2007 - February 2012
 */

#ifndef _SLRECTPIXELS_H_
#define _SLRECTPIXELS_H_


#include "slHistogram3ch.h"

#include <vector>
#include <math.h>
#include <iostream>


//!	This struct contain the info about a level of slRectPixels.
/*!
 *	\see		slRectSimple, slHistogram3ch
 *	\author		Michael Sills Lavoie
 *	\date		23.05.2007
 */
struct slLevelStat
{
	int RectangleHeight;
	int RectangleWidth;
	int numberOfRectangleX;
	int numberOfRectangleY;
};


//!	This struct contain the info about the texture of a slRectPixels.
/*!
 *	\see		slRectSimple, slHistogram3ch
 *	\author		Michael Sills Lavoie, Pier-Luc St-Onge
 *	\date		23.05.2007 - February 2012
 */
struct slTextureStat
{
	float mean_;
	float var_;
	std::vector<float> intensity_;
};


//!	slRectPixels used to store the histogram and other information.
/*!
 *	This class is used to seperate a frame into slRectPixels.
 *	A frame is composed of a set number of level of slRectPixels.
 *	The lower is a level, the bigger is the number of slRectPixels that it contain.
 *
 *	\see		slRectSimple, slHistogram3ch
 *	\author		Michael Sills Lavoie
 *	\date		23.05.2007
 */
class SLCORE_DLL_EXPORT slRectPixels
{
public:
	slRectPixels(int coordX, int coordY, bool isLowestLevel, int dimX, int dimY);	//!< Constructor
	~slRectPixels();	//!< Destructor

	void addLink(slRectPixels* rect1, slRectPixels* rect2, slRectPixels* rect3, slRectPixels* rect4); //!< Add four sub-rectangle-pixels
	void addExternalLink(slRectPixels* rect);	//!< Add other link betwen two slRectPixels

	void update(const slImage3ch &data, const slImage3ch &dataB);	//!< Update the histograms and the textures...

	void clearData(void);	//!< Reset object

	void compareRectangle (float th, float deltath, float thTexture, float dthTexture);	//!< Decides if background or not

	void setBackground (bool isBack);		//!< Force background
	bool getBackground (void) const;		//!< Returns true if background

	slHistogram3ch& getHistogram (void);	//!< Returns foreground's histogram
	slHistogram3ch& getHistogramB (void);	//!< Returns background's histogram

	int getCoordX (void) const;	//!< X offset in image
	int getCoordY (void) const;	//!< Y offset in image
	int getDimX (void) const;	//!< Width
	int getDimY (void) const;	//!< Height

	static void setTexture (bool useTexture);	//!< Force the use of textures

private:
	
	int coordX_;
	/*
	*    The y coord of the top left corner of the slRectPixels in the image
	*/
	int coordY_;
	/*
	*    The x dimention of the slRectPixels (in pixel)
	*/
	int dimX_;
	/*
	*    The y dimention of the slRectPixels (in pixel)
	*/
	int dimY_;
	/*
	*    The variance of intensity in this rectangle +
	*	 The intensity vector for this rectangle for the foreground data
	*/
	slTextureStat texture_;
	/*
	*    The variance of intensity in this rectangle +
	*	 The intensity vector for this rectangle for the background data
	*/
	slTextureStat textureB_;
	/*
	*    The histogram of this slRectPixels for the foreground data
	*/
	slHistogram3ch histogram_;
	/*
	*    The histogram of this slRectPixels for the background data
	*/
	slHistogram3ch histogramB_;
	/*
	*    A vector of pointers to the small slRectPixels contained in this slRectPixels.
	*	 (only a level lower)
	*/
	std::vector <slRectPixels*> contain_;
	/*
	*    A vector of pointers to the small slRectPixels that are at the border of this slRectPixels.
	*	 (only a level lower)
	*/
	std::vector <slRectPixels*> external_;
	/*
	*    True if the slRectPixels is at the lowest level (contain no other slRectPixels
	*/
	bool isLowestLevel_;
	/*
	*    True if this slRectPixels is in the background (no moving object detected)
	*/
	bool isBackground_;
	/*
	*    True if this slRectPixels have already been checked (in the same loop)
	*/
	bool isChecked_;
	/*
	*    True if we use the texture
	*/
	static bool useTexture_;
};


#endif //_SLRECTPIXELS_H_


