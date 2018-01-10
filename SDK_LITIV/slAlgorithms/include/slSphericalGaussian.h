/*!	\file	slSphericalGaussian.h
 *	\brief	This file contains the class slSphericalGaussian
 *
 *	\author		Pier-Luc St-Onge
 *	\date		February 2012
 */

// Adaptive background mixture models for real-time tracking
// Chris Stauffer, W.E.L Grimson
// http://www.ai.mit.edu/projects/vsam/Publications/stauffer_cvpr98_track.pdf

#ifndef SLSPHERICALGAUSSIAN_H
#define SLSPHERICALGAUSSIAN_H


#include "slAlgorithms.h"

#include <slCore.h>


//!	This class is a spherical Gaussian model
/*!
 *	There is only one common variance for a 3-channels model.
 *	The three channels are independent.
 *
 *	\see		slSpherGaussMixMat
 *	\author		Pier-Luc St-Onge
 *	\date		February 2012
 */
class SLALGORITHMS_DLL_EXPORT slSphericalGaussian
{
public:
	slSphericalGaussian(const cv::Vec3f &mean = cv::Vec3f(0, 0, 0), float variance = 1);	//!< Constructor

	void setMean(const cv::Vec3f &mean);	//!< Force new mean value vector
	const cv::Vec3f& getMean() const;		//!< Returns the current mean value vector

	void setVariance(float variance);		//!< Force new variance
	float getVariance() const;				//!< Returns the current variance

	float prob(const cv::Vec3f &X_t) const;	//!< Probability of X_t in the distribution

	void insertInlier(const cv::Vec3f &X_t, float alpha = 0.0625f);		//!< Update the distribution with X_t and learning rate alpha
	bool testInlier(const cv::Vec3f &X_t, float nbStdDev = 3.0f) const;	//!< Test if X_t is in the sphere of radius nbStdDev*sqrt(variance)

private:
	// Mean value for all components
	cv::Vec3f mean_;

	// Shared variance (spherical distribution)
	float variance_;

};



#endif	// SLSPHERICALGAUSSIAN_H


