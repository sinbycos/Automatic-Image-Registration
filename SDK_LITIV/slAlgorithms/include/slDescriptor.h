/*!	\file	slDescriptor.h
 *	\brief	This file contains classes slDescriptor,
 *			slDceK and slSkelRelDist
 *
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */

#ifndef SLDESCRIPTOR_H
#define SLDESCRIPTOR_H


#include "slAlgorithms.h"

#include <opencv2/core/core.hpp>
#include <string>


#define DESC_K "K"				//!< Descriptor name for DCE method
#define DESC_RELDIST "RelDist"	//!< Descriptor name for skeleton method


//!	This is the base class for all key points' descriptor
/*!
 *	This class is abstract.
 *
 *	The different descriptor names are defined in slKeyPoint.h.
 *
 *	\see		slDceK, slSkelRelDist, slKeyPoint, slBlobAnalyzer
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */
class SLALGORITHMS_DLL_EXPORT slDescriptor
{
public:
	slDescriptor();				//!< Constructor
	virtual ~slDescriptor();

	virtual slDescriptor* clone() const = 0;	//!< To get a clone of the descriptor
	virtual std::string getName() const = 0;	//!< To get the name of the descriptor

};


//!	This class represents the K value in DCE method.
/*!
 *	In the DCE method, the only descriptor for a key point is the k value.
 *	
 *	This class also implements some algorithms to compute the k value.
 *
 *	\see		slDce, slDescriptor, slKeyPoint, slKeyPoint.h
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */
class SLALGORITHMS_DLL_EXPORT slDceK: public slDescriptor
{
public:
	slDceK(float k = 0);					//!< Constructor

	virtual slDceK* clone() const;			//!< Generates a clone (calls copy constructor)
	virtual std::string getName() const;	//!< Returns DESC_K (see slKeyPoint.h)

	// Score function
	float scoreK(const slDceK &k2) const;	//!< Comparison function between two k values (negative normalized difference).

	static float compute(const cv::Point &p0, const cv::Point &p1, const cv::Point &p2);		//!< Computes k value, integer components
	static float compute(const cv::Point2f &p0, const cv::Point2f &p1, const cv::Point2f &p2);	//!< Computes k value, float components

public:
	float k_;	//!< The k value, read and write public access

};


//!	This class represents the relative distance value for the skeleton method
/*!
 *	In the skeleton method, the descriptor for a key point is the normalized
 *	distance between the point inside a blob and the closest point on the
 *	contour of that blob.
 *	In other words, the distance transform applied to a given blob is normalized
 *	to values limited from 0.0 to 1.0.
 *	So, the relative distance of a key point is only the value of the
 *	normalized distance transform for that key point which is inside the blob.
 *	
 *	This descriptor of a key point on the skeleton should be resistant
 *	to scaling and rotation of the blob.
 *
 *	\see		slSkel, slDescriptor, slKeyPoint, slKeyPoint.h
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */
class SLALGORITHMS_DLL_EXPORT slSkelRelDist: public slDescriptor
{
public:
	slSkelRelDist(float relDist = 0);		//!< Constructor

	virtual slSkelRelDist* clone() const;	//!< Generates a clone (calls copy constructor)
	virtual std::string getName() const;	//!< Returns DESC_RELDIST (see slKeyPoint.h)

	// Score function
	float scoreRelDist(const slSkelRelDist &rd2) const;	//!< Comparison function between two relative distances (negative absolute difference).

public:
	float relDist_;		//!< The relative distance, read and write public access

};


#endif	// SLDESCRIPTOR_H


