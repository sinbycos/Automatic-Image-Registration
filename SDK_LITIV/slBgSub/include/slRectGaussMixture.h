/*!	\file	slRectGaussMixture.h
 *	\brief	Remake of rafik background subtractor with a mix of the
 *			GaussianMixture bgs
 *
 *	\author		Michael Sills Lavoie, (GaussMixture) Pier-Luc St-Onge
 *	\date		30.05.2007 - February 2012
 */

#ifndef _SLRECTGAUSSMIXTURE_H_
#define _SLRECTGAUSSMIXTURE_H_


#include "slRectSimple.h"
#include "slSpherGaussMixMat.h"


//!	slRectGaussMixture based on rafik bg subtractor and slGaussMixture
/*!
 *	Foreground pixels detected by the slRectSimple algorithm are filtered
 *	a second time by a gaussian mixture algorithm.
 *	This hybrid algorithm uses similar parameters as the ones in
 *	slRectSimple and slGaussMixture on the command line.
 *
 *	\section constructor Constructor and Factory
 *	It is possible to create an instance directly:
 *	\code
 *	slRectGaussMixture rectGaussMixture;
 *	// Configuration...
 *	slSpherGaussMixMat &sgmm = rectGaussMixture.getGaussMixtures();
 *	\endcode
 *
 *	With slBgSubFactory, it is also possible to create an instance of slRectGaussMixture:
 *	\code
 *	slBgSub *bgSub = slBgSubFactory::createInstance("rectGaussMixture");
 *	// Delete bgSub
 *	\endcode
 *
 *	\see		slBgSub, slRectSimple, slSpherGaussMixMat, slRectPixels, slHistogram3ch
 *	\author		Michael Sills Lavoie, (GaussMixture) Pier-Luc St-Onge
 *	\date		30.05.2007 - February 2012
 */
class SLBGSUB_DLL_EXPORT slRectGaussMixture: public slRectSimple
{
public:
	slRectGaussMixture();			//!< Constructor
	virtual ~slRectGaussMixture();	//!< Destructor

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


class SLBGSUB_DLL_EXPORT slRectGaussMixtureFactory: public slBgSubFactory
{
public:
	virtual ~slRectGaussMixtureFactory();

protected:
	// To specify your functions's parameters
	virtual void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const;

	// The factory (a static instance) will create an instance of your BgSubtractor
	virtual slRectGaussMixture* createInstance() const;

private:
	// The factory's constructor
	slRectGaussMixtureFactory();

private:
	static slRectGaussMixtureFactory factory_;

};


#endif	// _SLRECTGAUSSMIXTURE_H_


