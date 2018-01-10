/*!	\file	slRectSimple.h
 *	\brief	Remake of rafik background subtractor
 *
 *	\author		Michael Sills Lavoie, Pier-Luc St-Onge
 *	\date		24.05.2007 - February 2012
 */

#ifndef _SLRECTSIMPLE_H_
#define _SLRECTSIMPLE_H_


#include "slArgHandler.h"
#include "slBgSub.h"
#include "slRectPixels.h"


//!	slRectSimple based on rafik bg subtractor.
/*!
 *	\section constructor Constructor and Factory
 *	It is possible to create an instance directly:
 *	\code
 *	slRectSimple rectSimple;
 *	// Configuration...
 *	\endcode
 *
 *	With slBgSubFactory, it is also possible to create an instance of slRectSimple:
 *	\code
 *	slBgSub *bgSub = slBgSubFactory::createInstance("rect");
 *	// Delete bgSub
 *	\endcode
 *
 *	\see		slBgSub, slRectGaussMixture, slRectPixels, slHistogram3ch
 *	\author		Michael Sills Lavoie, Pier-Luc St-Onge
 *	\date		24.05.2007 - February 2012
 */
class SLBGSUB_DLL_EXPORT slRectSimple: public slBgSub
{
public:
	slRectSimple();				//!< Constructor
	virtual ~slRectSimple();	//!< Destructor

	static void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap);

	// Set function(s)

	void setBaseEps(float th);			//!< Base epsilon for comparison (histogram)
	void setDeltaEps(float deltath);	//!< Delta to increase base epsilon (histogram) when going to lower level

	void setMinRect(int minRect);		//!< Minimal number of slRectPixels in the upper level

	void setTextureBaseEps(float Tth);			//!< Base epsilon for comparison (rectangle texture)
	void setTextureDeltaEps(float Tdeltath);	//!< Delta to increase base epsilon (rectangle texture) when going to lower level

	void setTexture(bool useTexture);	//!< Use the texture method
	void setDistanceMethod(bool mpda);	//!< Use the MDPA distance for the histograms

	void setColorSystem(typeColorSys colorSystem);	//!< SL_BGR or SL_HSV
	void setQuantification(bool enabled, const slQuant3ch& quant = slQuant3ch());	//!< Image quantification

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

protected:
	/**
	*	Container for the statistic of each level
	*/
	std::vector<slLevelStat> statistic_;

	/**
	*	Container of the level of slRectPixels for the current frame
	*	Each level contain all the slRectPixels of this level
	*/
	std::vector<std::vector<slRectPixels*> > level_;

	/**
	*	Th for the upper level slRectPixels when we compare their histograms
	*/
	float th_;
	/**
	*	The value that we add to the th_ for each lower level
	*/
	float deltath_;
	/**
	*	Minimal number of slRectPixels that are at the upper level
	*/
	int minRect_;

	/**
	*	Bool to if we use the MDPA distance for the histograms
	*/
	bool MPDA_;

	/**
	*	Bool to know if we use texture
	*/
	bool useTexture_;

	/**
	*	Treshold for the texture to know if a rectangle is textured or not
	*/
	float Tth_;

	/**
	*	The value that we add to the Tth_ for each lower level
	*/
	float Tdeltath_;

	void createLevel (std::vector<std::vector<slRectPixels*> > &level);
};


class SLBGSUB_DLL_EXPORT slRectSimpleFactory: public slBgSubFactory
{
public:
	virtual ~slRectSimpleFactory();

protected:
	// To specify your functions's parameters
	virtual void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const;

	// The factory (a static instance) will create an instance of your BgSubtractor
	virtual slRectSimple* createInstance() const;

private:
	// The factory's constructor
	slRectSimpleFactory();

private:
	static slRectSimpleFactory factory_;

};


#endif	// _SLRECTSIMPLE_H_


