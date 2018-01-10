/*!	\file	slContourEngine.h
 *	\brief	This file contains class slContourEngine.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		November 2011
 */

#ifndef SLCONTOURENGINE_H
#define SLCONTOURENGINE_H


#include "slAlgorithms.h"

#include <slContours.h>
#include <slArgHandler.h>


//!	This class adds additionnal functionalities to slContours
/*!
 *	This class can find contours on a binary image (slImage1ch), but
 *	according to its configuration, it can also preprocess the input
 *	image before finding the contours.
 *	For example, setClosure() can enable a closure on the image
 *	before slContours::findAll() is called.
 *
 *	\see		slContours, slBlobAnalyzer
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */
class SLALGORITHMS_DLL_EXPORT slContourEngine
{
public:
	slContourEngine();			//!< Constructor
	virtual ~slContourEngine();

	static void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap);	//!< To get the possible parameters

	// Set Functions

	void setParameters(const slAH::slParameters& parameters);		//!< To set the parameters

	void setClosure(bool enabled, int w = 3, int h = 3);			//!< To enable the closure filter and set the kernel size

	void showParameters() const;									//!< To show the parameters for the closure

	// Compute functions

	void findContours(slImage1ch &bForeground);		//!< Does a closure on bForeground if needed, then calls slContours::findAll()

	static slContour approximate(const slContour &contour, double distance);	//!< Contour approximation, distance is the maximum error of approximation

	// Get Functions

	const slContours& getContours() const { return contours_; }					//!< Returns found contours

private:
	bool doClosure_;
	cv::Mat kernel_;

	slContours contours_;

};


#endif	// SLCONTOURENGINE_H


