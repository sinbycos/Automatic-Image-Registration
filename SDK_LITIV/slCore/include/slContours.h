/*!	\file	slContours.h
 *	\brief	This file contains the definition of slContour and slContours
 *
 *	\author		Pier-Luc St-Onge
 *	\date		April 2011
 */

#ifndef SLCONTOURS_H
#define SLCONTOURS_H


#include "slCore.h"

#include <vector>


//!	Single contour made of points, no hierarchy
typedef std::vector<cv::Point> slContour;

class slContours;
class slContours_iterator;
class slContours_const_iterator;


//!	This class is the regular iterator for slContours objects
/*!
 *	This iterator has a reference to the slContours object and
 *	an index to one of its slContour.
 *	Like a C pointer, the slContour is accessible by using the
 *	operator*() or the operator->() for slContour's methods.
 *	There is also a mat() function to get the generic cv::Mat header.
 *	Finally, isNull() tells you if you have gone beyond a limit
 *	of the hierarchy of the slContours.
 *
 *	\see		slContours for examples, slContour, slContours_const_iterator
 *	\author		Pier-Luc St-Onge
 *	\date		April 2011
 */
class SLCORE_DLL_EXPORT slContours_iterator
{
public:
	slContours_iterator(slContours *ref = NULL, int index = 0);	//!< Default constructor
	friend class slContours_const_iterator;

	slContours_iterator next() const;		//!< Next (right) contour
	slContours_iterator previous() const;	//!< Previous (left) contour
	slContours_iterator child() const;		//!< Child (down) contour
	slContours_iterator parent() const;		//!< Parent (up) contour

	bool isNull() const;		//!< True if index < 0 or if slContours* is NULL

	cv::Mat mat();				//!< Returns explicitely a cv::Mat header for slContour
	slContour& operator*();		//!< Returns the instance of slContour
	slContour* operator->();	//!< Returns the slContour*

	bool operator==(const slContours_iterator&) const;		//!< Same index and same slContours*
	bool operator<(const slContours_iterator &it) const;	//!< For for and while loops, use isNull() instead

private:
	slContours *ref_;
	int index_;

};


//!	This class is the const iterator for slContours objects
/*!
 *	This iterator has a reference to the slContours object and
 *	an index to one of its slContour.
 *	Like a C pointer, the slContour is accessible by using the
 *	operator*() or the operator->() for slContour's methods.
 *	There is also a mat() function to get the generic cv::Mat header.
 *	Finally, isNull() tells you if you have gone beyond a limit
 *	of the hierarchy of the slContours.
 *
 *	\see		slContours for examples, slContour, slContours_iterator
 *	\author		Pier-Luc St-Onge
 *	\date		April 2011
 */
class SLCORE_DLL_EXPORT slContours_const_iterator
{
public:
	slContours_const_iterator(const slContours *ref = NULL, int index = 0);	//!< Default constructor
	slContours_const_iterator(const slContours_iterator &it);				//!< Const-cast constructor

	slContours_const_iterator next() const;		//!< Next (right) contour
	slContours_const_iterator previous() const;	//!< Previous (left) contour
	slContours_const_iterator child() const;	//!< Child (down) contour
	slContours_const_iterator parent() const;	//!< Parent (up) contour

	bool isNull() const;		//!< True if index < 0 or if slContours* is NULL

	const cv::Mat mat() const;				//!< Returns explicitely a cv::Mat header for slContour
	const slContour& operator*() const;		//!< Returns the instance of slContour
	const slContour* operator->() const;	//!< Returns the slContour*

	bool operator==(const slContours_const_iterator&) const;	//!< Same index and same slContours*
	bool operator<(const slContours_const_iterator &it) const;	//!< For for and while loops, use isNull() instead

private:
	const slContours *ref_;
	int index_;
};


//!	This class contains an hierarchy of slContour instances
/*!
 *	This class contains a vector of slContour objects.
 *	The hierarchy of these slContour objects is also defined in
 *	another vector: next, previous, child and parent contour.
 *	The goal of this class is to replace the old but efficient
 *	CvContour.
 *
 *	The contours are accessible and browsable by the iterators.  Example:
 *	\code
 *	slImage1ch grayScaleImage = grayClone(rgbImage);
 *	slContours contours;
 *
 *	contours.findAll(grayScaleImage);
 *
 *	for (slContours::const_iterator itContour = contours.begin(); !itContour.isNull(); itContour = itContour.next())
 *	{
 *		double area = fabs(contourArea(itContour.mat()));
 *	
 *		// Substract holes' area
 *		for (slContours::const_iterator child = itContour.child(); !child.isNull(); child = child.next())
 *		{
 *			area -= fabs(contourArea(child.mat()));
 *		}
 *	
 *		area = (area >= 0 ? area : 0);
 *	}
 *	\endcode
 *
 *	\see		slContour, slContours_iterator, slContours_const_iterator
 *	\author		Pier-Luc St-Onge
 *	\date		April 2011
 */
class SLCORE_DLL_EXPORT slContours
{
public:
	typedef class slContours_iterator iterator;				//!< Iterator type
	typedef class slContours_const_iterator const_iterator;	//!< Const iterator type

	friend class slContours_iterator;
	friend class slContours_const_iterator;

public:
	slContours();							//!< Default constructor, empty vectors
	slContours(const slContour &contour);	//!< Fills contours and hierarchy with this unique contour

	void clear();						//!< Clears both vectors (contours and hierarchy)
	void findAll(slImage1ch &image);	//!< Calls \c cv::findContours() with \c CV_RETR_CCOMP and \c CV_CHAIN_APPROX_SIMPLE

	iterator begin();				//!< Returns an iterator at index 0 or a null iterator
	const_iterator begin() const;	//!< Returns an iterator at index 0 or a null iterator

	std::vector<slContour>& all() { return contours_; }			//!< To get the contours
	std::vector<cv::Vec4i>& hierarchy() { return hierarchy_; }	//!< To get the hierarchy

private:
	std::vector<slContour> contours_;
	std::vector<cv::Vec4i> hierarchy_;

};


#endif	// SLCONTOURS_H


