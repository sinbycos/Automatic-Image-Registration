/*!	\file	slKeyPoint.h
 *	\brief	This file contains class slKeyPoint
 *				and some defines
 *
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */

#ifndef SLKEYPOINT_H
#define SLKEYPOINT_H


#include "slDescriptor.h"

#include <opencv2/core/core.hpp>
#include <vector>


typedef std::vector<cv::Point2f> CvPt2fVector_t;	//!< Vector of points, float components
typedef std::vector<cv::Point> CvPtVector_t;		//!< Vector of points, integer components


//!	This class is the key point
/*!
 *	A key point has a position (float components only),
 *	neighbor key points and one or many descriptors.
 *
 *	\see		slBlobAnalyzer, slDescriptor, slKeyPoint.h
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */
class SLALGORITHMS_DLL_EXPORT slKeyPoint
{
public:
	slKeyPoint();						//!< Default constructor
	slKeyPoint(const slKeyPoint &kPt);	//!< Copy constructor (calls operator=)

	~slKeyPoint();

	const slKeyPoint& operator=(const slKeyPoint &kPt);	//!< Copies attributes, clones descriptors

	void moveTo(const cv::Point2f &position);	//!< Sets the position of the key point
	const cv::Point2f& position() const;		//!< Returns the position of the key point

	void insert(const cv::Point &neighbor);		//!< Saves a neighbor point, integer components
	void insert(const cv::Point2f &neighbor);	//!< Saves a neighbor point, float components

	CvPt2fVector_t::const_iterator beginNeighbors() const;	//!< First neighbor
	CvPt2fVector_t::const_iterator endNeighbors() const;	//!< After last neighbor

	const slDescriptor* find(const std::string &name) const;	//!< Returns requested descriptor or NULL
	void insert(slDescriptor *desc);							//!< Saves a descriptor
	void remove(const std::string &name);						//!< Removes a descriptor

	// Score functions
	float scoreEuclidean(const slKeyPoint &kPt2, float diagLength,
		int offsetX = 0, int offsetY = 0) const;			//!< Comparison function, distance between two key points
	float scoreOrientation(const slKeyPoint &kPt2) const;	//!< Comparison function, orientation of neighbors of two key points
	float scoreNbNeighbors(const slKeyPoint &kPt2) const;	//!< Comparison function, number of neighbors of two key points

private:
	void clearDescriptors();

private:
	cv::Point2f position_;
	CvPt2fVector_t neighbors_;

	std::map<std::string, slDescriptor*> descriptors_;

};


//!	This is a functor to compare two points of type Point2f
/*!
 *	Useful for sorting points.
 *
 *	\see		slBlobAnalyzer for an example of use, slKeyPoint
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */
struct SLALGORITHMS_DLL_EXPORT CvPoint2fLessThan
{
	/*! The comparison operator
	 *	Compares two points.
	 */
	bool operator()(const cv::Point2f &p1, const cv::Point2f &p2) const
	{
		if (p1.y < p2.y) return true;
		if (p1.y > p2.y) return false;
		return (p1.x < p2.x);
	}
};


typedef std::map<cv::Point2f, slKeyPoint, CvPoint2fLessThan> slKeyPoints;	//!< Key points container


#endif	// SLKEYPOINT_H


