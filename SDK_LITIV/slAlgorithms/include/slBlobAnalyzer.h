/*!	\file	slBlobAnalyzer.h
 *	\brief	This file contains classes
 *			slBlobAnalyzer, slDce and slSkel.  There are also
 *			some factory classes for the previous classes.
 *
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */

#ifndef SLBLOBANALYZER_H
#define SLBLOBANALYZER_H


#include "slKeyPoint.h"

#include <slContours.h>
#include <slArgHandler.h>
//#include <slWindow.h>


#define ARG_BA "-ba"	//!< Blob analyzer argument on command line


///////////////////////////////////////////////////////////////////////////////
//	slBlobAnalyzer
///////////////////////////////////////////////////////////////////////////////


//!	Abstract class for all blob analyzers
/*!
 *	This class has the common interface for all blob analyzers.
 *
 *	Here is a complete example:
 *	\code
 *	slArgProcess argProcess;
 *	slArgHandler argHbgSub("bgSub"), argHContour("contour"), argHBlobAn("blobAn");
 *	
 *	// Get all possible parameters
 *	argProcess
 *		.addGlobal(slParamSpec("-i", "Video source", MANDATORY) << slSyntax("video.avi"))
 *		.addGlobal(slParamSpec("-h", "Help"));
 *	slBgSub::fillAllParamSpecs(argHbgSub);
 *	slContourEngine::fillParamSpecs(argHContour);
 *	slBlobAnalyzer::fillAllParamSpecs(argHBlobAn);
 *	
 *	argProcess << argHbgSub << argHContour << argHBlobAn;
 *	
 *	slVideoIn videoIn;
 *	slBgSub *bgSub = NULL;
 *	slContourEngine *contourEngine = new slContourEngine;
 *	slBlobAnalyzer *ba = NULL;
 *	slWindow winGraph("Graph from contours");
 *	
 *	try {
 *		// Load parameters from command line
 *		argProcess.parse(argc, argv);
 *		const slParameters &globalParams = argProcess.getParameters("");
 *	
 *		if (globalParams.isParsed("-h")) {	// Help wanted
 *			argProcess.printUsage();
 *			return 0;
 *		}
 *	
 *		// Configure all compute nodes and others
 *		videoIn.open(globalParams.getValue("-i").c_str());
 *		bgSub = slBgSubFactory::createInstance(argProcess.getParameters("bgSub"));
 *		contourEngine->setParameters(argProcess.getParameters("contour"));
 *		ba = slBlobAnalyzerFactory::createInstance(argProcess.getParameters("blobAn"));
 *	
 *		// Show configuration
 *		bgSub->showParameters();
 *		contourEngine->showParameters();
 *		ba->showParameters();
 *	}
 *	catch (const slExceptionArgHandler &err)
 *	{
 *		argProcess.printUsage();
 *		fprintf(stderr, "Error: %s\n", err.getMessage());
 *		return -1;
 *	}
 *	catch (const slException &err)
 *	{
 *		fprintf(stderr, "Error: %s\n", err.getMessage());
 *		return -1;
 *	}
 *	
 *	// Start main process
 *	try {
 *		slClock myClock;
 *		slImage3ch imSource;
 *		slImage1ch bForeground;
 *	
 *		myClock.setFPS(videoIn.getFPS());
 *		myClock.start();
 *	
 *		for (unsigned int ind = 0; ind < videoIn.getNbImages(); ind++) {
 *			videoIn.read(imSource);
 *			bgSub->compute(imSource, bForeground);
 *			contourEngine->findContours(bForeground);
 *			ba->analyzeAllBlobs(contourEngine->getContours());
 *	
 *			paint(bForeground, contourEngine->getContours(), ba, imSource);
 *			winGraph.show(imSource);
 *			waitKey(myClock.nextDelay());
 *		}
 *	
 *		myClock.stop();
 *	}
 *	catch (const slException &err)
 *	{
 *		fprintf(stderr, "Error: %s\n", err.getMessage());
 *		return -1;
 *	}
 *	
 *	videoIn.close();
 *	delete ba;
 *	delete contourEngine;
 *	delete bgSub;
 *	\endcode
 *
 *	The function \c paint() is defined as:
 *	\code
 *	void paint(const slImage1ch &fg, const slContours &contours, const slBlobAnalyzer *ba, slImage3ch &output)
 *	{
 *		// Empty output image
 *		output.create(fg.size());
 *		output = 0;
 *	
 *		// For each external contour
 *		for (slContours::const_iterator contour = contours.begin();
 *			!contour.isNull(); contour = contour.next())
 *		{
 *			if (ba->hasKeyPoints(contour)) {
 *				paintKeyPoints(ba->getKeyPoints(contour), CV_RGB(0, 255, 0), output);
 *	
 *				// For each hole or internal contour
 *				for (slContours::const_iterator child = contour.child();
 *					!child.isNull(); child = child.next())
 *				{
 *					if (ba->hasKeyPoints(child)) {
 *						paintKeyPoints(ba->getKeyPoints(child), CV_RGB(255, 0, 0), output);
 *					}
 *				}
 *			}
 *		}
 *	}
 *	\endcode
 *
 *	Finally, the function \c paintKeyPoints() is defined as:
 *	\code
 *	void paintKeyPoints(const slKeyPoints &kPt, cv::Scalar color, slImage3ch &output)
 *	{
 *		// For all key points of the contour
 *		for (slKeyPoints::const_iterator itPoint = kPt.begin();
 *			itPoint != kPt.end(); itPoint++)
 *		{
 *			// For all neighbor key points of itPoint
 *			for (CvPt2fVector_t::const_iterator itNb = itPoint->second.beginNeighbors();
 *				itNb != itPoint->second.endNeighbors(); itNb++)
 *			{
 *				// Only paint one line between itPoint and itNb
 *				if (CvPoint2fLessThan()(itPoint->first, *itNb)) {
 *					line(output, Point(itPoint->first), Point(*itNb), color);
 *				}
 *			}
 *		}
 *	}
 *	\endcode
 *
 *	\see		slBlobAnalyzerFactory, slDce, slSkel, slContourEngine, slContours, slKeyPoints, CvPt2fVector_t
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */
class SLALGORITHMS_DLL_EXPORT slBlobAnalyzer
{
public:
	slBlobAnalyzer(bool analyzeHoles = false);	//!< Constructor.  By default, the holes are not analyzed
	virtual ~slBlobAnalyzer();

	static void fillAllParamSpecs(slAH::slParamSpecMap& paramSpecMap);		//!< All parameter spec. for all blob analyzers
	static void fillGlobalParamSpecs(slAH::slParamSpecMap& paramSpecMap);	//!< General common parameters only

	// Set Functions

	void setParameters(const slAH::slParameters& parameters);	//!< Complete configuration of the blob analyzer

	void setMinArea(double minArea);	//!< Minimum area to analyze a blob

	void showParameters() const;		//!< Write configuration to STDOUT

	// Compute functions

	void analyzeAllBlobs(const slContours &contours);								//!< Analyzes all contours individually
	virtual slKeyPoints analyzeBlob(const slContours::const_iterator &contour) = 0;	//!< Pure virtual analyze function

	virtual float compareKeyPoints(const slKeyPoint &kPt1, const slKeyPoint &kPt2,
		float diagLength, int offsetX = 0, int offsetY = 0) const = 0;				//!< kPt1.xy + offset.xy vs kPt2.xy, abstract function

	// Get Functions

	bool hasKeyPoints(const slContours::const_iterator &contour) const;					//!< Returns true if contour has key points
	const slKeyPoints& getKeyPoints(const slContours::const_iterator &contour) const;	//!< Returns the key points for that contour

protected:
	// Set specific parameters
	virtual void setSubParameters(const slAH::slParameters& parameters) = 0;

	// Shows (with cout) you function's parameters' getValue
	virtual void showSubParameters() const = 0;

protected:
	std::map<slContours::const_iterator, slKeyPoints> points_;

private:
	bool analyzeHoles_;
	double minArea_;

};


//!	This is the blob analyzer for the DCE method
/*!
 *	Discrete Curve Evolution removes iteratively the less significant
 *	point on the contour until the maximum number of remaining key
 *	points is reached.
 *	One should use setMaxNumOfPoints() to set that maximum number of key points.
 *
 *	\see		slBlobAnalyzer, slDceK, slKeyPoints, slContours
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */
class SLALGORITHMS_DLL_EXPORT slDce: public slBlobAnalyzer
{
public:
	slDce();			//!< Constructor
	virtual ~slDce();

	static void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap);		//!< Adds slDce parameters in the specification

	// Set function(s)

	void setMaxNumOfPoints(int maxPt);									//!< Number of key points to keep on the final contour

	// Compute functions

	slKeyPoints analyzeBlob(const slContours::const_iterator &contour);	//!< The main compute fonction for DCE method

	virtual float compareKeyPoints(const slKeyPoint &kPt1, const slKeyPoint &kPt2,
		float diagLength, int offsetX = 0, int offsetY = 0) const;		//!< Returns a global comparison score for DCE key points (the greater value the better)

protected:
	// Set specific parameters
	virtual void setSubParameters(const slAH::slParameters& parameters);

	// Shows (with cout) you function's parameters' getValue
	virtual void showSubParameters() const;

private:
	int maxPt_;

};


//!	This is the blob analyzer for the skeleton method
/*!
 *	First, the class tries to remove the noise on the blob's contour.
 *	Second, it applies a distance transform on that blob.
 *	Third, a kernel is applied in order to find points on that distance
 *	transformation that have a local high second derivate value.
 *	Fourth, these points are then linked together with the Prim's algorithm.
 *	Finally, the key points have one or at least three neighbor points.
 *
 *	The points that only have two neighbors have been removed from the
 *	returned key points.
 *	The remaining skeleton is finally made of relatively long straight lines.
 *
 *	\see		slBlobAnalyzer, slSkelRelDist, slKeyPoints, slContours
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */
class SLALGORITHMS_DLL_EXPORT slSkel: public slBlobAnalyzer
{
public:
	slSkel();			//!< Constructor
	virtual ~slSkel();

	static void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap);			//!< Adds slSkel parameters in the specification

	// Set function(s)

	void setSize(cv::Size size);											//!< Size of temporary image buffers (default: 320x240)
	void setMinHoleArea(unsigned int minHoleArea);							//!< Holes may need to have a significant size (default: 512 square pixels)
	void setApprox(bool doApprox, double extDist = 0, double intDist = 0);	//!< Contour approximation (default: ext. 3.5 pixels, int. 3.0 pixels)
	void setPeakThreshold(float peakTh);									//!< Threshold to identify points on distance transformed blob (default: 4.0)

	// Compute functions

	slKeyPoints analyzeBlob(const slContours::const_iterator &contour);		//!< The main compute fonction for skeleton method

	virtual float compareKeyPoints(const slKeyPoint &kPt1, const slKeyPoint &kPt2,
		float diagLength, int offsetX = 0, int offsetY = 0) const;	//!< Returns a global comparison score for skeleton key points (the greater value the better)

protected:
	// Set specific parameters
	virtual void setSubParameters(const slAH::slParameters& parameters);

	// Shows (with cout) you function's parameters' getValue
	virtual void showSubParameters() const;

private:
	typedef std::vector<unsigned int> UIntVector_t;

private:
	void distanceTransform(const slContours::const_iterator &contour, const cv::Rect &rect);
	CvPtVector_t getRawPoints(const slContours::const_iterator &contour, const cv::Rect &rect) const;
	unsigned int* createDistMat(const CvPtVector_t &rawPts) const;
	void computeParents(UIntVector_t &parents, const unsigned int *dists, unsigned int distInf) const;
	slKeyPoints computeKeyPoints(const CvPtVector_t &rawPts, UIntVector_t &parents) const;

private:
	cv::Size size_;
	slImage1ch imBlob_;
	slImage1fl imDist_;
	slImage1fl imSkel_;
	//slWindow winSkel_;

	unsigned int minHoleArea_;
	bool doApprox_;
	double extDist_, intDist_;
	float peekThreshold_;

	slImage1fl kernel181_;

};


///////////////////////////////////////////////////////////////////////////////
//	slBlobAnalyzerFactory
///////////////////////////////////////////////////////////////////////////////


//!	This class can create a slBlobAnalyzer instance
/*!
 *	There are two createInstance() functions.
 *
 *	\see		slBlobAnalyzer for an example, slDce, slSkel
 *	\author		Pier-Luc St-Onge
 *	\date		July 2011
 */
class SLALGORITHMS_DLL_EXPORT slBlobAnalyzerFactory
{
public:
	virtual ~slBlobAnalyzerFactory();

	static void fillAllParamSpecs(slAH::slParamSpecMap& paramSpecMap);

	static slBlobAnalyzer* createInstance(const char *bgSubName);					//!< Creates a slBlobAnalyzer by the name
	static slBlobAnalyzer* createInstance(const slAH::slParameters& parameters);	//!< Creates a slBlobAnalyzer by the command line arguments

protected:
	// The factory's constructor
	slBlobAnalyzerFactory(const std::string& name);

	// To specify your functions's parameters
	virtual void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const = 0;

	// The factory (a static instance) will create an instance of your BgSubtractor
	virtual slBlobAnalyzer* createInstance() const = 0;

protected:
	std::string name_;

private: // Types
	typedef std::map<std::string, slBlobAnalyzerFactory*> factories_t;	// List of static factories

private:
	static factories_t *factories_;

};


class SLALGORITHMS_DLL_EXPORT slDceFactory: public slBlobAnalyzerFactory
{
public:
	virtual ~slDceFactory();

protected:
	// To specify your functions's parameters
	virtual void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const;

	// The factory (a static instance) will create an instance of your BlobAnalyzer
	virtual slDce* createInstance() const;

private:
	// The factory's constructor
	slDceFactory();

private:
	static slDceFactory factory_;

};


class SLALGORITHMS_DLL_EXPORT slSkelFactory: public slBlobAnalyzerFactory
{
public:
	virtual ~slSkelFactory();

protected:
	// To specify your functions's parameters
	virtual void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const;

	// The factory (a static instance) will create an instance of your BlobAnalyzer
	virtual slSkel* createInstance() const;

private:
	// The factory's constructor
	slSkelFactory();

private:
	static slSkelFactory factory_;

};


#endif	// SLBLOBANALYZER_H


