/******************************************************************************
	File:	slSimpleGauss.cpp
	Description:	This is the implementation of the simple Gaussian class for
					all background subtractor algorithms.
	Created:	October 2006 (Atousa Torabi)
	Modified:	November 2006:
					Some comments (Atousa Torabi)
	
******************************************************************************/

#include "slSimpleGauss.h"

#include <iostream>
#include <omp.h>


using namespace cv;
using namespace std;
using namespace slAH;


// ARGUMENTS
#define ARG_SOBEL		"-g"
#define ARG_ALPHA	    "-alpha"
#define ARG_CAMInt	    "-sigmaInt"
#define ARG_CAMChr	    "-sigmaChr"
#define ARG_CoeffInt    "-coeffI"
#define ARG_CoeffChr	"-coeffC"
#define ARG_CoeffGrad	"-coeffG"

#define ARG_GRADX  "-gx" //gradient image for X
#define ARG_GRADY  "-gy" //gradient image for Y 


// Initialize some static parameters
float slSimpleGauss::slIntensityComp::mAlpha = (float)0.97;
float slSimpleGauss::slIntensityComp::mSigmaInt = (float)5.0;
float slSimpleGauss::slIntensityComp::mCoeffInt	 = (float)2.0;

float slSimpleGauss::slChromacityComp::mAlpha = (float)0.97;
float slSimpleGauss::slChromacityComp::mSigmaCh = (float)0.001;
float slSimpleGauss::slChromacityComp::mCoefCh = (float)5.0;

float slSimpleGauss::slGradientComp::mAlpha = (float)0.97;
float slSimpleGauss::slGradientComp::mCoefGrad = (float)5.0;



slSimpleGauss::slSimpleGauss()
: slBgSub(), mIntPixels(NULL), mChrPixels(NULL), mGradPixels(NULL)
{
	// Set parameters to default values

	setDoSobel(false);
	setAlpha(0.97f);

	setSigmaInt(5.0f);
	setSigmaChr(0.001f);

	setCoeffInt(2.0f);
	setCoeffChr(5.0f);
	setCoeffGrad(5.0f);
}


slSimpleGauss::~slSimpleGauss()
{
	delete [] mIntPixels;
	delete [] mChrPixels;
	delete [] mGradPixels;
}


void slSimpleGauss::fillParamSpecs(slParamSpecMap& paramSpecMap)
{
	paramSpecMap << (slParamSpec(ARG_SOBEL, "aperture size (sobel)") << slSyntax("3..9", "3"));  //gradient

	paramSpecMap
		<< (slParamSpec(ARG_ALPHA,		"Alpha: learning rate")	<< slSyntax("0..1",		"0.97"))
		<< (slParamSpec(ARG_CAMInt,		"Sigma intensity")		<< slSyntax("1..255",	"5"))
		<< (slParamSpec(ARG_CAMChr,		"Sigma chromacity")		<< slSyntax("0..1",		"0.001"))
		<< (slParamSpec(ARG_CoeffInt,	"Coeff. intensity")		<< slSyntax("1..20",	"2"))
		<< (slParamSpec(ARG_CoeffChr,	"Coeff. chromacity")	<< slSyntax("1..20",	"5"))
		<< (slParamSpec(ARG_CoeffGrad,	"Coeff. gradient")		<< slSyntax("1..20",	"5"));

	paramSpecMap << slParamSpec(ARG_GRADX, "Current gradientX frame"); //gradient
	paramSpecMap << slParamSpec(ARG_GRADY, "Current gradientY frame");
}


void slSimpleGauss::setDoSobel(bool enabled, int apSize)
{
	mDoSobel = enabled; // false; // Toujours false pour l'instant
	mApSize = apSize;
}


void slSimpleGauss::setAlpha(float alpha)
{
	mAlpha = alpha;
	slIntensityComp::setAlpha(mAlpha);
	slChromacityComp::setAlpha(mAlpha);
	slGradientComp::setAlpha(mAlpha);
}


void slSimpleGauss::setSigmaInt(float sigma)
{
    mSigmaIn = sigma;
	slIntensityComp::setSigma(mSigmaIn);
}


void slSimpleGauss::setSigmaChr(float sigma)
{
    mSigmaCh = sigma;
	slChromacityComp::setSigma(mSigmaCh);
}


void slSimpleGauss::setCoeffInt(float coeff)
{
    mCoeffInt = coeff;
	slIntensityComp::setCoeff(mCoeffInt);
}


void slSimpleGauss::setCoeffChr(float coeff)
{
    mCoeffChr = coeff;
	slChromacityComp::setCoef(mCoeffChr);
}


void slSimpleGauss::setCoeffGrad(float coeff)
{
    mCoeffGrad = coeff;
	slGradientComp::setCoef(mCoeffGrad);
}


void slSimpleGauss::setSubWindowEnabled(const char *arg, bool enable)
{
	windowsList_t::iterator itW = windowsList_.find(arg);

	if (enable == true) {	// Create window
		// If not found
		if (itW == windowsList_.end()) {
			if (strcmp(arg, ARG_GRADX) == 0) {
				windowsList_[arg] = new slWindow("SimpleGauss - Grad. X");
			}
			if (strcmp(arg, ARG_GRADY) == 0) {
				windowsList_[arg] = new slWindow("SimpleGauss - Grad. Y");
			}
		}
	}
	else {	// Delete window
		// If found
		if (itW != windowsList_.end()) {
			delete itW->second;
			windowsList_.erase(itW);
		}
	}
}


void slSimpleGauss::setSubParameters(const slParameters& parameters)
{
	if (parameters.isParsed(ARG_COLOR_S) &&
		strcmp(parameters.getValue(ARG_COLOR_S).c_str(), BGR_NAME) != 0)
	{
		throw slException("Simple gaussian only works in the rgb color space.");
	}

	// Gradients estimate
	setDoSobel(parameters.isParsed(ARG_SOBEL), atoi(parameters.getValue(ARG_SOBEL).c_str()));

	//Alpha
	setAlpha((float)atof(parameters.getValue(ARG_ALPHA).c_str()));

    //Sigma Intensity
	setSigmaInt((float)atof(parameters.getValue(ARG_CAMInt).c_str()));

	//Sigma chromacity
	setSigmaChr((float)atof(parameters.getValue(ARG_CAMChr).c_str()));

    //Coef Intensity
	setCoeffInt((float)atof(parameters.getValue(ARG_CoeffInt).c_str()));

	//Coef chromacity
	setCoeffChr((float)atof(parameters.getValue(ARG_CoeffChr).c_str()));

	//Coef gradient
	setCoeffGrad((float)atof(parameters.getValue(ARG_CoeffGrad).c_str()));

	// Set windows
	setSubWindowEnabled(ARG_GRADX, parameters.isParsed(ARG_GRADX));
	setSubWindowEnabled(ARG_GRADY, parameters.isParsed(ARG_GRADY));
}


void slSimpleGauss::showSubParameters() const
{
	cout << "--- slSimpleGauss ---" << endl;

	if (mDoSobel)
		cout << "Do sobel : yes -> " << mApSize << endl;
	else
		cout << "Do sobel : no" << endl;

	cout << "Alpha : " << mAlpha << endl;
	cout << "SigmaInt : " << mSigmaIn << endl;
	cout << "SigmaChr : " << mSigmaCh << endl;
	cout << "CoeffInt : " << mCoeffInt << endl;
	cout << "CoeffChr : " << mCoeffChr << endl;
	cout << "CoeffGrad : " << mCoeffGrad << endl;
}


void slSimpleGauss::init()
{
	if (colorSystem_ == SL_HSV) {
		throw slException("Simple gaussian only works in the rgb color space.");
	}

	const int w = imageSize_.width;
	const int h = imageSize_.height;

	delete [] mIntPixels;
	delete [] mChrPixels;
	delete [] mGradPixels;

	mIntPixels = new slIntensityComp[w * h];
	mChrPixels = new slChromacityComp[w * h];
	mGradPixels = new slGradientComp[w * h];

	if (mDoSobel)
	{
		// Create a version of current images
		mGradX = current_.clone();
		mGradY = current_.clone();

		doGradX(mGradX, mApSize);	// Estimate gradient X
		doGradY(mGradY, mApSize);	// Estimate gradient Y
	}

#pragma omp parallel for
	for (int i = 0; i < h; i++) {
		const int wi = w * i;

		const slPixel3ch* cur_row = background_[i];
		const slPixel3ch* grad_rowX = (mDoSobel ? mGradX[i] : NULL);
		const slPixel3ch* grad_rowY = (mDoSobel ? mGradY[i] : NULL);

		for (int j = 0; j < w; j++) {
			// Find intensity image mean and variance
			mIntPixels[wi + j] += cur_row[j];

			// Find chromacity image mean ang variance
			mChrPixels[wi + j] += cur_row[j]; 

			// Find gradient image mean and variance
			if (mDoSobel) {
				mGradPixels[wi + j].add(grad_rowX[j], grad_rowY[j]);
			}
		}
	}
}


void slSimpleGauss::doSubtraction(slImage1ch &bForeground)
{
	const int w = imageSize_.width;
	const int h = imageSize_.height;

	// Finding the average chromacity for all pixels of a frame

	float gAvgStdDev[3];

	if (mDoSobel) {
		float gAvgStdDevB = 0;
		float gAvgStdDevG = 0;
		float gAvgStdDevR = 0;

#pragma omp parallel for reduction(+ : gAvgStdDevB, gAvgStdDevG, gAvgStdDevR)
		for (int i = 0; i < h; i++) {
			const int wi = w * i;

			for (int j = 0; j < w; j++) {
				gAvgStdDevB += mGradPixels[wi + j].getVarianceB();
				gAvgStdDevG += mGradPixels[wi + j].getVarianceG();
				gAvgStdDevR += mGradPixels[wi + j].getVarianceR();
			}
		}

		gAvgStdDev[0] = sqrt(gAvgStdDevB / (w * h));
		gAvgStdDev[1] = sqrt(gAvgStdDevG / (w * h));
		gAvgStdDev[2] = sqrt(gAvgStdDevR / (w * h));
	}

	// Now, do the subtraction

	// Empty foreground images
	bForeground = PIXEL_1CH_BLACK;

#pragma omp parallel for
	for (int i = 0; i < h; i++) {
		const int wi = w * i;

		const slPixel3ch* cur_row = current_[i];
		slPixel3ch* bg_row = background_[i];

		slPixel1ch* b_fg_row = bForeground[i];
		const slPixel3ch* grad_rowX = (mDoSobel ? mGradX[i] : NULL);
		const slPixel3ch* grad_rowY = (mDoSobel ? mGradY[i] : NULL);

		for (int j = 0; j < w; j++) {
			// Foreground pixel
			if	(mDoSobel ? mChrPixels[wi + j].pixIsForground(cur_row[j]) ||
					mGradPixels[wi + j].pixIsForground(grad_rowX[j], grad_rowY[j], gAvgStdDev) :
					mIntPixels[wi + j].pixIsForeground(cur_row[j])
				)
			{
			    // Update binary foreground - foreground pixel
			    b_fg_row[j] = PIXEL_1CH_WHITE;
			}
			// Backgroung pixel 
			else {
				// Update intensity image mean and variance
				mIntPixels[wi + j] += cur_row[j];

				// Update chromacity image mean ang variance
				mChrPixels[wi + j] += cur_row[j];

				// Update gradient image mean ang variance
				if (mDoSobel) {
					mGradPixels[wi + j].add(grad_rowX[j], grad_rowY[j]);
				}

				// Update background pixel
				bg_row[j] = mIntPixels[wi + j].getPixel();
			}
		}
	}
}


void slSimpleGauss::setBgPixel(const slPixel3ch *cur_row,
	slPixel3ch *bg_row, slPixel1ch *b_fg_row, int w, int i, int j)
{
	const int wi = w * i;

	// Update intensity image mean and variance
	mIntPixels[wi + j] += cur_row[j];

	// Update chromacity image mean ang varinace
	mChrPixels[wi + j] += cur_row[j];

	// Update gradient image mean ang varinace
	if (mDoSobel) {
		const slPixel3ch* grad_rowX = mGradX[i];
		const slPixel3ch* grad_rowY = mGradY[i];

		mGradPixels[wi + j].add(grad_rowX[j], grad_rowY[j]);
	}

	// Update background pixel
	bg_row[j] = mIntPixels[wi + j].getPixel();

    // Update binary foreground - non foreground pixel
	b_fg_row[j] = PIXEL_1CH_BLACK;
}


void slSimpleGauss::prepareNextSubtraction()
{
	if (mDoSobel) {
		// Copy the current image
		current_.copyTo(mGradX);
		current_.copyTo(mGradY);

		doGradX(mGradX, mApSize);	// Estimate gradient X
		doGradY(mGradY, mApSize);	// Estimate gradient Y
	}
}


void slSimpleGauss::updateSubWindows()
{
	windowsList_t::iterator it;

	if ((it = windowsList_.find(ARG_GRADX)) != windowsList_.end()) {
		it->second->show(mGradX, colorSystem_);	// grad X
	}
	if ((it = windowsList_.find(ARG_GRADY)) != windowsList_.end()) {
		it->second->show(mGradY, colorSystem_);	// grad Y
	}
}


void slSimpleGauss::doGradX(slImage3ch &image, const int level)
{
	doGrad(image, 1, 0, level);
}


void slSimpleGauss::doGradY(slImage3ch &image, const int level)
{
	doGrad(image, 0, 1, level);
}


void slSimpleGauss::doGrad(slImage3ch &image, int xorder, int yorder, const int level)
{
	Mat_<Vec3s> image16;

	Sobel(image, image16, image16.depth(), xorder, yorder, level);
	convertScaleAbs(image16, image);
}


///////////////////////////////////////////////////////////////////////////////
//	slSimpleGauss::slChromacityComp
///////////////////////////////////////////////////////////////////////////////


slSimpleGauss::slChromacityComp::slChromacityComp()
: mNbPix(0), mcMeanG(0), mcMeanR(0), mcVarianceG(0), mcVarianceR(0)
{
}


//Find if a pixel is foreground or backgorund
bool slSimpleGauss::slChromacityComp::pixIsForground(const slPixel3ch &pixel) const
{
	float chromG = (float)pixel.val[1] / (pixel.val[0] + pixel.val[1] + pixel.val[2]);
	float chromR = (float)pixel.val[2] / (pixel.val[0] + pixel.val[1] + pixel.val[2]);

	return (
		abs(chromG - mcMeanG) > mCoefCh * max(mSigmaCh, sqrt(mcVarianceG)) ||
		abs(chromR - mcMeanR) > mCoefCh * max(mSigmaCh, sqrt(mcVarianceR)) );
}


//Find mean and variance for each chromacity pixel
const slSimpleGauss::slChromacityComp& slSimpleGauss::slChromacityComp::operator+=(const slPixel3ch &pixel)
{
	//Find chromacity for each pixel
	float chromG = (float)pixel.val[1] / (pixel.val[0] + pixel.val[1] + pixel.val[2]);
	float chromR = (float)pixel.val[2] / (pixel.val[0] + pixel.val[1] + pixel.val[2]);

	if (mNbPix > 0) { 
	    float mcPrevG = mcMeanG;
	    float mcPrevR = mcMeanR;

	    mcMeanG = mAlpha * mcPrevG + (1 - mAlpha) * chromG; 
	    mcMeanR = mAlpha * mcPrevR + (1 - mAlpha) * chromR; 

		mcVarianceG = mAlpha * (mcVarianceG + (mcMeanG - mcPrevG) * (mcMeanG - mcPrevG)) +
			(1 - mAlpha) * (chromG - mcMeanG) * (chromG - mcMeanG);

		mcVarianceR = mAlpha * (mcVarianceR + (mcMeanR - mcPrevR) * (mcMeanR - mcPrevR)) +
			(1 - mAlpha) * (chromR - mcMeanR) * (chromR - mcMeanR);
	}
	else {
	    mcMeanG = chromG; 
	    mcMeanR = chromR;
	}

   	mNbPix++;
	return *this;
}


void slSimpleGauss::slChromacityComp::setAlpha(const float alpha)
{
	mAlpha = alpha;
}


void slSimpleGauss::slChromacityComp::setSigma(const float sigma)
{
	mSigmaCh = sigma;
}


void slSimpleGauss::slChromacityComp::setCoef(const float coef)
{
	mCoefCh = coef;
}


///////////////////////////////////////////////////////////////////////////////
//	slSimpleGauss::slIntensityComp
///////////////////////////////////////////////////////////////////////////////


slSimpleGauss::slIntensityComp::slIntensityComp()
: mNbPix(0), mMeanB(0), mMeanG(0), mMeanR(0), mVarianceB(0), mVarianceG(0), mVarianceR(0)
{
}


// Find if a pixel is foreground or background
bool slSimpleGauss::slIntensityComp::pixIsForeground(const slPixel3ch &pixel) const
{
	return (
		abs(pixel.val[0] - mMeanB) > mCoeffInt * sqrt(mVarianceB) ||
		abs(pixel.val[1] - mMeanG) > mCoeffInt * sqrt(mVarianceG) ||
		abs(pixel.val[2] - mMeanR) > mCoeffInt * sqrt(mVarianceR) );
}


//Find mean and variance for intensity pixel
const slSimpleGauss::slIntensityComp& slSimpleGauss::slIntensityComp::operator+=(const slPixel3ch &pixel)
{
	float mPrevB, mPrevG, mPrevR;

	if (mNbPix > 0) { 
		mPrevB = mMeanB;
		mPrevG = mMeanG;
		mPrevR = mMeanR;
	}
	else {
		mPrevB = pixel.val[0]; 
	    mPrevG = pixel.val[1]; 
	    mPrevR = pixel.val[2];

		mVarianceR = mVarianceG = mVarianceB = mSigmaInt * mSigmaInt;
	}

	mMeanB = mAlpha * mPrevB + (1 - mAlpha) * pixel.val[0];
    mMeanG = mAlpha * mPrevG + (1 - mAlpha) * pixel.val[1];
    mMeanR = mAlpha * mPrevR + (1 - mAlpha) * pixel.val[2]; 

	mVarianceB = mAlpha * (mVarianceB + (mMeanB - mPrevB) * (mMeanB - mPrevB)) +
		(1 - mAlpha) * (pixel.val[0] - mMeanB) * (pixel.val[0] - mMeanB);

	mVarianceG = mAlpha * (mVarianceG + (mMeanG - mPrevG) * (mMeanG - mPrevG)) +
		(1 - mAlpha) * (pixel.val[1] - mMeanG) * (pixel.val[1] - mMeanG);

	mVarianceR = mAlpha * (mVarianceR + (mMeanR - mPrevR) * (mMeanR - mPrevR)) +
		(1 - mAlpha) * (pixel.val[2] - mMeanR) * (pixel.val[2] - mMeanR);

	mNbPix++;	
	return *this;
}


const slPixel3ch slSimpleGauss::slIntensityComp::getPixel() const
{
	return slPixel3ch((int)mMeanB, (int)mMeanG, (int)mMeanR);
}


void slSimpleGauss::slIntensityComp::setAlpha(const float alpha)
{
	mAlpha = alpha;
}


void slSimpleGauss::slIntensityComp::setSigma(const float sigma)
{
	mSigmaInt = sigma;
}


void slSimpleGauss::slIntensityComp::setCoeff(const float coeff)
{
	mCoeffInt = coeff;
}


///////////////////////////////////////////////////////////////////////////////
//	slSimpleGauss::slGradientComp
///////////////////////////////////////////////////////////////////////////////


slSimpleGauss::slGradientComp::slGradientComp()
: mNbPix(0),
mgxMeanB(0), mgxMeanG(0), mgxMeanR(0),
mgyMeanB(0), mgyMeanG(0), mgyMeanR(0),  
mgVarianceB(0), mgVarianceG(0), mgVarianceR(0)
{
}


bool slSimpleGauss::slGradientComp::pixIsForground(
	const slPixel3ch &pixelX, const slPixel3ch &pixelY, float avgStdDev[3]) const
{
	return (
		sqrt(	(pixelX.val[0] - mgxMeanB) * (pixelX.val[0] - mgxMeanB) +
				(pixelY.val[0] - mgyMeanB) * (pixelY.val[0] - mgyMeanB) ) > mCoefGrad * max(avgStdDev[0], sqrt(mgVarianceB)) ||
		sqrt(	(pixelX.val[1] - mgxMeanG) * (pixelX.val[1] - mgxMeanG) +
				(pixelY.val[1] - mgyMeanG) * (pixelY.val[1] - mgyMeanG) ) > mCoefGrad * max(avgStdDev[1], sqrt(mgVarianceG)) ||
		sqrt(	(pixelX.val[2] - mgxMeanR) * (pixelX.val[2] - mgxMeanR) +
				(pixelY.val[2] - mgyMeanR) * (pixelY.val[2] - mgyMeanR) ) > mCoefGrad * max(avgStdDev[2], sqrt(mgVarianceR)) );
}


void slSimpleGauss::slGradientComp::add(const slPixel3ch &pixelX , const slPixel3ch &pixelY)
{
	if (mNbPix > 0) { 
	    float mgxPrevB = mgxMeanB;
	    float mgxPrevG = mgxMeanG;
	    float mgxPrevR = mgxMeanR;

	    float mgyPrevB = mgyMeanB;
	    float mgyPrevG = mgyMeanG;
	    float mgyPrevR = mgyMeanR;

		float mgxvarPrevB = mgxVarianceB;
	    float mgxvarPrevG = mgxVarianceG;
	    float mgxvarPrevR = mgxVarianceR;

	    float mgyvarPrevB = mgyVarianceB;;
	    float mgyvarPrevG = mgyVarianceB;;
	    float mgyvarPrevR = mgyVarianceB;;

	    mgxMeanB = mAlpha * mgxPrevB + (1 - mAlpha) * pixelX.val[0];
	    mgxMeanG = mAlpha * mgxPrevG + (1 - mAlpha) * pixelX.val[1];
	    mgxMeanR = mAlpha * mgxPrevR + (1 - mAlpha) * pixelX.val[2];

		mgyMeanB = mAlpha * mgyPrevB + (1 - mAlpha) * pixelY.val[0];
        mgyMeanG = mAlpha * mgyPrevG + (1 - mAlpha) * pixelY.val[1];
	    mgyMeanR = mAlpha * mgyPrevR + (1 - mAlpha) * pixelY.val[2];

		mgxVarianceB = mAlpha * (mgxvarPrevB + (mgxMeanB - mgxPrevB) * (mgxMeanB - mgxPrevB)) +
			(1 - mAlpha) * (pixelX.val[0] - mgxMeanB) * (pixelX.val[0] - mgxMeanB);
		mgxVarianceG = mAlpha * (mgxvarPrevG + (mgxMeanG - mgxPrevG) * (mgxMeanG - mgxPrevG)) +
			(1 - mAlpha) * (pixelX.val[1] - mgxMeanG) * (pixelX.val[1] - mgxMeanG);
		mgxVarianceR = mAlpha * (mgxvarPrevR + (mgxMeanR - mgxPrevR) * (mgxMeanR - mgxPrevR)) +
			(1 - mAlpha) * (pixelX.val[2] - mgxMeanR) * (pixelX.val[2] - mgxMeanR);

	    mgyVarianceB = mAlpha * (mgyvarPrevB + (mgyMeanB - mgyPrevB) * (mgyMeanB - mgyPrevB)) +
			(1 - mAlpha) * (pixelY.val[0] - mgyMeanB) * (pixelY.val[0] - mgyMeanB);
		mgyVarianceG = mAlpha * (mgyvarPrevG + (mgyMeanG - mgyPrevG) * (mgyMeanG - mgyPrevG)) +
			(1 - mAlpha) * (pixelY.val[1] - mgyMeanG) * (pixelY.val[1] - mgyMeanG);
		mgyVarianceR = mAlpha * (mgyvarPrevR + (mgyMeanR - mgyPrevR) * (mgyMeanR - mgyPrevR)) +
			(1 - mAlpha) * (pixelY.val[2] - mgyMeanR) * (pixelY.val[2] - mgyMeanR);

		mgVarianceB = sqrt(mgxVarianceB * mgxVarianceB + mgyVarianceB * mgyVarianceB);
		mgVarianceG = sqrt(mgxVarianceG * mgxVarianceG + mgyVarianceG * mgyVarianceG);
		mgVarianceR = sqrt(mgxVarianceR * mgxVarianceR + mgyVarianceR * mgyVarianceR);
	}
	else {
		mgxMeanB = pixelX.val[0]; 
	    mgxMeanG = pixelX.val[1]; 
	    mgxMeanR = pixelX.val[2];

		mgyMeanB = pixelY.val[0]; 
	    mgyMeanG = pixelY.val[1]; 
	    mgyMeanR = pixelY.val[2];
	}

	mNbPix++;	
}


void slSimpleGauss::slGradientComp::setAlpha(const float alpha)
{
	mAlpha = alpha;
}


void slSimpleGauss::slGradientComp::setCoef(const float coef)
{
	mCoefGrad = coef;
}


///////////////////////////////////////////////////////////////////////////////
//	slSimpleGaussFactory
///////////////////////////////////////////////////////////////////////////////


slSimpleGaussFactory slSimpleGaussFactory::factory_;


slSimpleGaussFactory::slSimpleGaussFactory()
: slBgSubFactory("simpleGauss")
{
}


slSimpleGaussFactory::~slSimpleGaussFactory()
{
}


void slSimpleGaussFactory::fillParamSpecs(slParamSpecMap& paramSpecMap) const
{
	slSimpleGauss::fillParamSpecs(paramSpecMap);
}


slSimpleGauss* slSimpleGaussFactory::createInstance() const
{
	return new slSimpleGauss();
}


