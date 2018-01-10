/*!	\file	slBgSub.h
 *	\brief	Contains the base class for background subtractors
 *
 *	This file contains the definition of the class slBgSub.
 *
 *	\author		Pier-Luc St-Onge, Michael Eilers-Smith
 *	\date		June 2010
 */

#ifndef _SLBGSUB_H
#define _SLBGSUB_H


#ifndef SLBGSUB_DLL_EXPORT
	#ifdef WIN32
		#define SLBGSUB_DLL_EXPORT __declspec(dllexport)
	#else
		#define SLBGSUB_DLL_EXPORT
	#endif
#endif


#include <slArgHandler.h>
#include <slContours.h>
#include <slQuantParams.h>
#include <slWindow.h>


// arguments (commands)
#define ARG_ALGO	"-a"	//!< for algorithm, doSubtraction()

#define ARG_SMOOTH	"-s"	//!< for image smoothing before subtraction

#define ARG_COLOR_S	"-c"	//!< for choice of color space (RGB or HSV)
#define ARG_QUANT	"-q"	//!< for quantification after smoothing

#define ARG_LIGHT_C	"-l"	//!< for light changes consideration

#define ARG_SHADOW_FILTER	"-sf"	//!< for shadow filter after subtraction
#define ARG_BLOB_FILTER		"-bf"	//!< for blob size filter after shadow filter
#define ARG_HOLE_FILTER		"-hf"	//!< for hole size filter after shadow filter

#define ARG_C_FR	"-cf"	//!< show current frame
#define ARG_QC_FR	"-qcf"	//!< show quantified current frame

#define ARG_BG		"-bg"	//!< show background
#define ARG_QBG		"-qbg"	//!< show quantified background

#define ARG_FG		"-fg"	//!< show foreground
#define ARG_BFG		"-bfg"	//!< show binary foreground


//!	This is the base class for all background subtractors
/*!
 *	Classes slTempAvg (Temporal Averaging), slSimpleGauss (Simple Gaussian)
 *	and slGaussMixture (Gaussian Mixture) share the same interface which
 *	is defined in the current class.
 *
 *	\section slBgSub_constructors Constructors and Factories
 *	The current class is abstract, so we cannot create a slBgSub directly.
 *	As seen in derived classes, it is possible to create a background
 *	subtractor directly:
 *	\code
 *	slTempAvg tempAvg;
 *	slSimpleGauss simpleGauss;
 *	slGaussMixture gaussMixture;
 *	slRectSimple rectSimple;
 *	slRectGaussMixture rectGaussMixture;
 *	// Configuration...
 *	\endcode
 *
 *	If the type of background subtractor is only known at runtime, then
 *	we could use the factory:
 *	\code
 *	slBgSub *bgSub1 = slBgSubFactory::createInstance("tempAVG");
 *	slBgSub *bgSub2 = slBgSubFactory::createInstance("simpleGauss");
 *	slBgSub *bgSub3 = slBgSubFactory::createInstance("gaussMixture");
 *	slBgSub *bgSub4 = slBgSubFactory::createInstance("rect");
 *	slBgSub *bgSub5 = slBgSubFactory::createInstance("rectGaussMixture");
 *	// Delete all bgSub*...
 *	\endcode
 *
 *	\section slBgSub_configuration Configuration
 *	The direct configuration functions are:
 *	- setColorSystem(): you can choose to work in BGR or HSV color space
 *	- setConsiderLightChanges(): only in HSV color space, if false, the V
 *		component has no effect on the result of the test
 *	- setSmooth(): to smooth the image to a specific level
 *	- setQuantification(): to quantify the image, one precision for each component
 *	- setShadowFilter(): to remove shadow pixels from the foreground
 *	- setSizeFilter(): to remove small blobs, some blobs exist because of noise in video
 *	- setWindowEnabled(): to enable individually the current image, the foreground image, etc.
 *
 *	Example:
 *	\code
 *	bgSub->setSizeFilter(true, 64);
 *	bgSub->setWindowEnabled(ARG_C_FR);	// Show current image or frame
 *	bgSub->setWindowEnabled(ARG_FG);	// Show foreground image
 *	\endcode
 *
 *	Note: global \c ARG values are defined in slBgSub.h.
 *
 *	It is also possible to configure the slBgSub instance with a slAH::slParameters object:
 *	\code
 *	bgSub->setParameters(argHandler.getParameters());
 *	\endcode
 *	Finally, it is possible to create and configure a background subtractor on the same line:
 *	\code
 *	slArgHandler argHandler("bgSub");
 *
 *	slBgSub::fillAllParamSpecs(argHandler);
 *	argHandler.parse(argc, argv);
 *	slBgSub *bgSub = slBgSubFactory::createInstance(argHandler.getParameters());
 *	\endcode
 *	Note: global usage for all algorithms can be printed to \c STDOUT this way:
 *	\code
 *	argHandler.printUsage();
 *	\endcode
 *
 *	\section slBgSub_compute Execute Background Subtraction One Image at a Time
 *	The compute() function receives an image and returns the result in
 *	the provided binary foreground image:
 *	\code
 *	slVideoIn videoSource;	
 *	slImage3ch currentImage;
 *	slImage1ch binForeground;
 *	
 *	videoSource.open("file.avi");
 *	videoSource.read(currentImage);
 *	bgSub->compute(currentImage, binForeground);
 *	\endcode
 *
 *	\section slBgSub_results Output of the Background Subtractor
 *	There are many informations we can get from the background subtractor:
 *	- getContours(): the contour of all blobs in the final binary foreground image
 *	- getBackground(): the background image
 *	- getForeground(): the foreground image with a white background
 *
 *	\see		slTempAvg, slSimpleGauss, slGaussMixture, slRectSimple, slRectGaussMixture
 *	\see		slImage3ch, slImage1ch
 *	\author		Pier-Luc St-Onge, Michael Eilers-Smith
 *	\date		May 2011
 */
class SLBGSUB_DLL_EXPORT slBgSub
{
	//friend class slTestComparaison;

public:
	slBgSub();
	virtual ~slBgSub();

	// To fill parameters' specification

	static void fillAllParamSpecs(slAH::slParamSpecMap& paramSpecMap);	//!< To get the syntax of all parameters of all algorithms
	static void fillGlobalParamSpecs(slAH::slParamSpecMap& paramSpecMap);

	// Set Functions

	void setParameters(const slAH::slParameters& parameters);	//!< Complete configuration of the background subtractor

	virtual void setColorSystem(typeColorSys colorSystem);		//!< SL_BGR or SL_HSV
	void setConsiderLightChanges(bool enabled);					//!< In HSV mode, disable test on V

	void setSmooth(bool enabled, int level = 0);				//!< Image smoothing
	virtual void setQuantification(bool enabled, const slQuant3ch& quant = slQuant3ch());	//!< Image quantification

	void setShadowFilter(bool enabled, double th = 0.3, double ts = 0.4, double tv = 0.2);	//!< Remove false positives due to shadows
	void setSizeFilter(bool enabled, int minBlobSize = 0, int minHoleSize = 0);	//!< Remove small blobs from foreground

	void setWindowEnabled(const char *arg, bool enable = true);	//!< Activate some windows: foreground, background, etc.

	void showParameters() const;	//!< Write configuration to STDOUT

	// Compute functions

	void compute(const slImage3ch &image, slImage1ch &bForeground);	//!< Do the background subtraction, BGR image

	// Get Functions

	const slContours& getContours() const;			//!< Returns the final contour structure

	slImage3ch& getCurrent();						//!< Current image
	const slImage3ch& getCurrent() const;			//!< Current image
	slImage3ch& getCurrentQuant();					//!< Quantified current image
	const slImage3ch& getCurrentQuant() const;		//!< Quantified current image

	slImage3ch& getBackground();					//!< Background image
	const slImage3ch& getBackground() const;		//!< Background image
	slImage3ch& getBackgroundQuant();				//!< Quantified background image
	const slImage3ch& getBackgroundQuant() const;	//!< Quantified background image

	slImage3ch& getForeground();					//!< Foreground image
	const slImage3ch& getForeground() const;		//!< Foreground image

	//CvSize getSize() const;
	//int getNbFrames() const;

protected:
	// The algorithm constructors

	//-----------------------------------------------------------------------
	// All the following functions must be implemented in your BgSubtractor:

	// Set specific parameters
	virtual void setSubParameters(const slAH::slParameters& parameters) = 0;

	// Shows (with cout) you function's parameters' getValue
	virtual void showSubParameters() const = 0;

	// For things to do before computing the first current frame
	virtual void init() = 0;

	// The function that actually computes the current frame
	virtual void doSubtraction(slImage1ch &bForeground) = 0;

	// To set a specific background pixel
	virtual void setBgPixel(const slPixel3ch *cur_row,
		slPixel3ch *bg_row, slPixel1ch *b_fg_row, int w, int i, int j) = 0;

	// For things to do before computing the current image
	virtual void prepareNextSubtraction() = 0;

	// Update any other windows
	virtual void updateSubWindows() = 0;

protected:
	typedef std::map<std::string, slWindow*> windowsList_t;

protected:
	//-----------
	// Parameters
	//-----------

	typeColorSys colorSystem_;

	bool doSmooth_;
	int smoothLevel_;

	bool doQuantification_;
	slQuant3ch quantParams_;

	bool doConsiderLightChanges_;

	bool doShadowFilter_;
	double th_, ts_, tv_;

	bool doSizeFilter_;
	int minBlobSize_;
	int minHoleSize_;

	//------------------------
	// Other shared attributes
	//------------------------

	// Because of this, it is not possible to create
	// your own algorithm outside the current project
	cv::Size imageSize_;

	slImage3ch current_;		// current frame
	slImage3ch qCurrent_;		// quantified current frame

	slImage3ch background_;		// background
	slImage3ch qBackground_;	// quantified background

	slImage3ch foreground_;		// foreground

	windowsList_t windowsList_;

private:
	void shadowFilter(slImage1ch &bForeground);

	void findBlobs(slImage1ch &bForeground);
	double getAreaSurface(const slContours::const_iterator &contour);
	void copyBlobIntoBG(const slContours::const_iterator &contour, slImage1ch &bForeground);
	void copyHoleIntoFG(const slContours::const_iterator &contour, slImage1ch &bForeground);

	void updateWindows(const slImage1ch &bForeground);

private: // Internal attributes
	int nbFrames_;

	slContours contours_;

};


class SLBGSUB_DLL_EXPORT slBgSubFactory
{
public:
	virtual ~slBgSubFactory();

	static void fillAllParamSpecs(slAH::slParamSpecMap& paramSpecMap);

	static slBgSub* createInstance(const char *bgSubName);
	static slBgSub* createInstance(const slAH::slParameters& parameters);

protected:
	// The factory's constructor
	slBgSubFactory(const std::string& name);

	// To specify your functions's parameters
	virtual void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const = 0;

	// The factory (a static instance) will create an instance of your BgSubtractor
	virtual slBgSub* createInstance() const = 0;

protected:
	std::string name_;

private: // Types
	typedef std::map<std::string, slBgSubFactory*> factories_t;	// List of static factories

private:
	// Because of this, it is not possible to create
	// your own algorithm outside the current project
	static factories_t *factories_;

};


#endif	// _SLBGSUB_H


