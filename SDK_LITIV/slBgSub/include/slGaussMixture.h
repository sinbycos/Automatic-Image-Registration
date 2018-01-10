/*!	\file	slGaussMixture.h
 *	\brief	Contains the class slGaussMixture, which is the Gaussian Mixture
 *			background subtraction class
 *
 *	This file contains the definition of the class slGaussMixture and
 *	its corresponding factory.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		June 2010
 */

#ifndef _SLGAUSSMIXTURE_H_
#define _SLGAUSSMIXTURE_H_


#include "slBgSub.h"
#include "slSpherGaussMixMat.h"


//!	This is the class for Gaussian Mixture background subtraction
/*!
 *	The current class does background subtraction by comparing each pixel of a
 *	new image to its corresponding multiple Gaussian distributions of the
 *	background.
 *	In fact, for each pixel, there are many Gaussian distributions to describe
 *	different colors that could be part of the background.  For example: waves.
 *
 *	In this algorithm, all pixels are saved in one of the multiple Gaussian
 *	distributions, but background pixels are the ones that belong to the most
 *	popular distributions (highest weights).
 *
 *	\section constructor Constructor and Factory
 *	It is possible to create an instance directly:
 *	\code
 *	slGaussMixture gaussMixture;
 *	// Configuration...
 *	slSpherGaussMixMat &sgmm = gaussMixture.getGaussMixtures();
 *	\endcode
 *
 *	With slBgSubFactory, it is also possible to create an instance of slGaussMixture:
 *	\code
 *	slBgSub *bgSub = slBgSubFactory::createInstance("gaussMixture");
 *	// Delete bgSub
 *	\endcode
 *
 *	\see		slBgSub, slSpherGaussMixMat, slTempAvg, slSimpleGauss
 *	\author		Pier-Luc St-Onge
 *	\date		June 2010
 */
class SLBGSUB_DLL_EXPORT slGaussMixture: public slBgSub
{
public:
	slGaussMixture();			//!< Constructor
	virtual ~slGaussMixture();	//!< Destructor

	static void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap);

	// Get function(s)

	//! Returns the slSpherGaussMixMat instance
	inline slSpherGaussMixMat& getGaussMixtures() { return gaussMixtures_; }

protected:
	// Set specific parameters
	virtual void setSubParameters(const slAH::slParameters& parameters);

	// Shows (with cout) you function's parameters' getValue
	virtual void showSubParameters() const;

	// For things to do before computing the first current frame
	virtual void init();

	// The function that actually computes the current frame
	virtual void doSubtraction(slImage1ch &bForeground);

	// To set a specific background pixel
	virtual void setBgPixel(const slPixel3ch *cur_row,
		slPixel3ch *bg_row, slPixel1ch *b_fg_row, int w, int i, int j);

	// For things to do before computing the current image
	virtual void prepareNextSubtraction();

	// Update any other windows
	virtual void updateSubWindows();

private:
	// A Gaussian mixture for all pixels
	slSpherGaussMixMat gaussMixtures_;

};


class SLBGSUB_DLL_EXPORT slGaussMixtureFactory: public slBgSubFactory
{
public:
	virtual ~slGaussMixtureFactory();

protected:
	// To specify your functions's parameters
	virtual void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const;

	// The factory (a static instance) will create an instance of your BgSubtractor
	virtual slGaussMixture* createInstance() const;

private:
	// The factory's constructor
	slGaussMixtureFactory();

private:
	static slGaussMixtureFactory factory_;

};


#endif	// _SLGAUSSMIXTURE_H_


