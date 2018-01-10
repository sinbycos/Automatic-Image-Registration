/*!	\file	slSpherGaussMixMat.h
 *	\brief	This file contains the class slSpherGaussMixMat
 *
 *	\author		Pier-Luc St-Onge
 *	\date		February 2012
 */

// Adaptive background mixture models for real-time tracking
// Chris Stauffer, W.E.L Grimson
// http://www.ai.mit.edu/projects/vsam/Publications/stauffer_cvpr98_track.pdf

#ifndef SLSPHERGAUSSMIXMAT_H
#define SLSPHERGAUSSMIXMAT_H


#include "slSphericalGaussian.h"

#include <slArgHandler.h>


//!	This class is a vector of vectors of K slSphericalGaussian instances
/*!
 *	This class and its private sub-classes implement the Gaussian Mixture Model
 *	algorithm.
 *	It is a vector of Gaussian mixtures, which are simply a vector of up to K
 *	sorted instances of slSphericalGaussian.
 *	While it is implemented as a vector, one could still resize it to the total
 *	number of pixels in an image.
 *
 *	The Gaussian Mixture parameters are set once, and they are used for all
 *	mixtures.  There are five of these and five corresponding "set" methods.
 *
 *	All new Gaussian distributions are initiated with a default variance.
 *	The current class does not know anything about the domain of computed
 *	values, so the "default variance" is 1 by default, and it should be set to
 *	a proper value since it has some impact on the behavior of the mixture.
 *
 *	\see		slSphericalGaussian, slGaussMixture, slRectGaussMixture
 *	\author		Pier-Luc St-Onge
 *	\date		February 2012
 */
class SLALGORITHMS_DLL_EXPORT slSpherGaussMixMat
{
public:
	slSpherGaussMixMat();

	static void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap);	//!< To get the parameters' syntax

	// Set functions

	void setK(size_t K);				//!< K distributions (default = 3)
	void setDefVariance(float defVar);	//!< Default variance for new distributions (default = 1.0)
	void setDistWidth(float nbStdDev);	//!< Distribution width (# std dev.) (default = 2.5)
	void setAlpha(float alpha);			//!< Learning rate (default = 0.05)
	void setT(float T);					//!< T threshold : Sum(w_i, 0..B-1) > T, (default = 0.5)

	void setParameters(const slAH::slParameters& parameters);	//!< Complete configuration
	void showParameters() const;								//!< Show parameters' value

	// Compute functions

	void reset(size_t nbMixtures);								//!< Global reset on the matrix
	bool update(size_t index, const cv::Vec3f &X_t);			//!< Returns true if X_t matches a gaussian, then updates the mixture

	// Get functions

	size_t getMixtureSize(size_t index) const;					//!< Returns number of activated gaussians
	float getWeight(size_t index, size_t k = 0) const;			//!< Returns the distribution's weight
	const cv::Vec3f& getMean(size_t index, size_t k = 0) const;	//!< Returns the mean of a distribution
	float getVariance(size_t index, size_t k = 0) const;		//!< Returns the variance of a distribution

private:
	class slSpherGaussMix
	{
	public:
		slSpherGaussMix();

		void reset(size_t K);					//!< Creates K slSphericalGaussian, none is activated

		bool update(const cv::Vec3f &X_t, float variance,
			float distW, float alpha, float T);	//!< Returns true if X_t matches a gaussian, updates the gaussian mixture

		size_t getSize() const;					//!< Returns number of activated slSphericalGaussian
		float getWeight(size_t k) const;		//!< Returns the distribution's weight
		const slSphericalGaussian& operator[](size_t k) const;

	private:
		struct slWeightedSpherGauss
		{
			bool operator<(const slWeightedSpherGauss& wsg) const;

			float weight;
			slSphericalGaussian *gaussian;
		};

	private:
		std::vector<slSphericalGaussian> sphericalGaussians_;
		std::vector<slWeightedSpherGauss> gaussMixture_;

		size_t K_;
		size_t B_;
	};

private:
	size_t K_;

	float defaultVariance_;
	float distWidth_;
	float alpha_;
	float T_;

	std::vector<slSpherGaussMix> gaussMixtures_;

};


#endif	// SLSPHERGAUSSMIXMAT_H


