/*!	\file	slTempAvg.h
 *	\brief	Contains the class slTempAvg, which is the Temporal Averaging
 *			background subtraction class
 *
 *	This file contains the definition of the class slTempAvg and
 *	its corresponding factory.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		June 2010
 */

#ifndef _SLTEMPAVG_H_
#define _SLTEMPAVG_H_


#include "slBgSub.h"
#include "slEpsilon3ch.h"


//!	This is the class for Temporal Averaging background subtraction
/*!
 *	The current class does background subtraction by comparing each pixel of a
 *	new image to its corresponding pixel in the static background image.
 *	This last image is made of the mean of all corresponding background pixels
 *	in computed images.
 *
 *	\section constructor Constructor and Factory
 *	It is possible to create an instance directly:
 *	\code
 *	slTempAvg tempAvg;
 *	// Configuration...
 *	\endcode
 *
 *	With slBgSubFactory, it is also possible to create an instance of slTempAvg:
 *	\code
 *	slBgSub *bgSub = slBgSubFactory::createInstance("tempAVG");
 *	// Delete bgSub
 *	\endcode
 *
 *	\see		slBgSub, slSimpleGauss, slGaussMixture
 *	\author		Pier-Luc St-Onge
 *	\date		June 2010
 */
class SLBGSUB_DLL_EXPORT slTempAvg: public slBgSub
{
public:
	slTempAvg();
	virtual ~slTempAvg();

	static void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap);

	// Set function(s)

	void setEpsilon(int eps);	//!< Epsilon or error tolerance (default = 15)

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
	class MeanPixel3ch
	{
	public:
		MeanPixel3ch();

		const MeanPixel3ch& operator+=(const slPixel3ch &pixel);

		slPixel3ch getMeanPixel() const;

	private:
		int sumB_;
		int sumG_;
		int sumR_;

		int nbPix_;
	};

private:
	slEpsilon3ch epsilon_;

	MeanPixel3ch* meanPix_;

};


class SLBGSUB_DLL_EXPORT slTempAvgFactory: public slBgSubFactory
{
public:
	virtual ~slTempAvgFactory();

protected:
	// To specify your functions's parameters
	virtual void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const;

	// The factory (a static instance) will create an instance of your BgSubtractor
	virtual slTempAvg* createInstance() const;

private:
	// The factory's constructor
	slTempAvgFactory();

private:
	static slTempAvgFactory factory_;

};


#endif	// _SLTEMPAVG_H_


