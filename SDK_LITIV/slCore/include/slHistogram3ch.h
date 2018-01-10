/*!	\file	slHistogram3ch.h
 *	\brief	Histogram used in rectangle background subtractor
 *
 *	\author		Michael Sills Lavoie, Pier-Luc St-Onge
 *	\date		23.05.2007 - February 2012
 */

#ifndef _SLHISTOGRAM3CH_H_
#define _SLHISTOGRAM3CH_H_


#include <vector>

#include "slCore.h"


//!	Histogram used in Rafik background subtractor.
/*!
 *	The background subtractor algorithm is implemented in slRectSimple.
 *	\see		slRectPixels, slRectSimple
 *	\author		Michael Sills Lavoie, Pier-Luc St-Onge
 *	\date		February 2012
 */
class SLCORE_DLL_EXPORT slHistogram3ch
{
public:
	slHistogram3ch();		//!< Constructor
	~slHistogram3ch();		//!< Destructor

	static void setRange(int range1,int range2, int range3);	//!< Static fonction used to set the range of each histogram
	static void setColorSpace(bool HSV);						//!< HSV or not
	static void setDistanceMethod(bool MPDA);					//!< MPDA distance or not

	void clear(void);			//!< Reset or empty all histograms
	void normalize (void);		//!< Normalize the sum to 1.0

	bool isEmpty (void) const;	//!< Returns true if empty

	slHistogram3ch& operator += (const slHistogram3ch& right);	//!< Add other histograms' values
	slHistogram3ch& operator += (const slPixel3ch& right);		//!< Add a pixel's channels to the histograms

	std::vector<float> compare (const slHistogram3ch& right) const;

private:

	/*
	*Upper limit of the histogram.
	*Highest value possible for a pixel single chanel after quantification.
	*/
	static int RANGE1;
	/*
	*Upper limit of the histogram.
	*Highest value possible for a pixel single chanel after quantification.
	*/
	static int RANGE2;
	/*
	*Upper limit of the histogram.
	*Highest value possible for a pixel single chanel after quantification.
	*/
	static int RANGE3;
	/*
	*Define the color space used in the histogram.
	*True is HSV.
	*/
	static bool HSV_;
	/*
	*Define the distance computation method used to check the distance of the histogram.
	*True is MPDA (Supposed to be more precised.
	*/
	static bool MPDA_;
	/*
	*Histogram that contain the first chanel.
	*/
	std::vector<float> ch1_;
	/*
	*Histogram that contain the first chanel.
	*/
	std::vector<float> ch2_;
	/*
	*Histogram that contain the first chanel
	*/
	std::vector<float> ch3_;
	/*
	*Used to know if the histograms are empty.
	*/
	bool isEmpty_;
};


#endif //_SLHISTOGRAM3CH_H_


