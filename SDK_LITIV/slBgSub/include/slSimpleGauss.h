/*!	\file	slSimpleGauss.h
 *	\brief	Contains the class slSimpleGauss, which is the Simple Gaussian
 *			background subtraction class
 *
 *	This file contains the definition of the class slSimpleGauss and
 *	its corresponding factory.
 *
 *	\author		Pier-Luc St-Onge, Atousa Torabi
 *	\date		June 2010
 */

#ifndef _SLSIMPLEGAUSS_H_
#define _SLSIMPLEGAUSS_H_


#include <windows.h> 

#include "slBgSub.h"


//!	This is the class for Simple Gaussian background subtraction
/*!
 *	The current class does background subtraction by comparing each pixel of a
 *	new image to its corresponding Gaussian distribution of the background.
 *	This algorithm is flexible, since new background pixels are used to update
 *	their corresponding Gaussian distribution.  In other words, it should be
 *	able to support slow light changes.
 *
 *	\section constructor Constructor and Factory
 *	It is possible to create an instance directly:
 *	\code
 *	slSimpleGauss simpleGauss;
 *	// Configuration...
 *	\endcode
 *
 *	With slBgSubFactory, it is also possible to create an instance of slSimpleGauss:
 *	\code
 *	slBgSub *bgSub = slBgSubFactory::createInstance("simpleGauss");
 *	// Delete bgSub
 *	\endcode
 *
 *	\see		slBgSub, slTempAvg, slGaussMixture
 *	\author		Pier-Luc St-Onge, Atousa Torabi
 *	\date		June 2010
 */
class SLBGSUB_DLL_EXPORT slSimpleGauss: public slBgSub
{
public:
	slSimpleGauss();
	virtual ~slSimpleGauss();

	static void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap);

	// Set function(s)

	void setDoSobel(bool enabled, int apSize = 3);	//!< Applies or not the Sobel filter, default kernel size is 3
	void setAlpha(float alpha);						//!< Global alpha value (Learning rate parameter) (default = 0.97)

	void setSigmaInt(float sigma);		//!< Camera variance parameter for intensity (default = 5)
	void setSigmaChr(float sigma);		//!< Camera variance parameter for chromacity (default = 0.001)

	void setCoeffInt(float coeff);		//!< Coefficient for Standard deviation intensity (default = 2)
	void setCoeffChr(float coeff);		//!< Coefficient for Standard deviation chromacity (default = 5)
	void setCoeffGrad(float coeff);		//!< Coefficient for Standard deviation gradient (default = 5)

	void setSubWindowEnabled(const char *arg, bool enable = true);	//!< Activate windows for gradient images: "-gx", "-gy"

	static void doGradX(slImage3ch &image, const int level);		//!< Applies a horizontal gradient filter
	static void doGradY(slImage3ch &image, const int level);		//!< Applies a vertical gradient filter
	static void doGrad(slImage3ch &image,
		int xorder, int yorder, const int level);	//!< Applies a gradient filter, any X and Y order

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
    //==================================================================
	//	This class contains mean and variance for each chromacity pixel .
	//==================================================================
	class slChromacityComp
	{
	public:
        slChromacityComp();

        //Find if a pixel is forground or backgorund
		bool pixIsForground(const slPixel3ch &pixel) const;

		//Find mean and variance for each chromacity pixel
		const slChromacityComp& operator+=(const slPixel3ch &pixel);
		
		static void setAlpha(const float alpha);
		static void setSigma(const float sigma);
		static void setCoef(const float coef);

	private:
        int  mNbPix;
		float mcMeanG;
		float mcMeanR;

		float mcVarianceG;
		float mcVarianceR;

        //Learning rate parameter
		static float mAlpha;

		//Camera variance parameter for chromacity
	    static float mSigmaCh;

		//Coefficient for Standard deviation chromacity
	    static float mCoefCh;
	   
	};
        
private:
	//==================================================================
	//	This class contains mean and variance for each intensity pixel .
	//==================================================================
	class slIntensityComp
	{
	public:
		slIntensityComp();

		// Find if a pixel is foreground or background
		bool pixIsForeground(const slPixel3ch &pixel) const;

        //Find mean and variance for intensity pixel
		const slIntensityComp& operator+=(const slPixel3ch &pixel);

		const slPixel3ch getPixel() const;

		static void setAlpha(const float alpha);
		static void setSigma(const float sigma);
		static void setCoeff(const float coeff);

	private:
        int  mNbPix;

		float mMeanB;
		float mMeanG;
		float mMeanR;

		float mVarianceB;
		float mVarianceG;
		float mVarianceR;

		// Learning rate parameter
		static float mAlpha;

		// Camera variance parameter for intensity
		static float mSigmaInt;

		// Coefficient for Standard deviation intensity
	    static float mCoeffInt;
	  
	};

private:
	//==================================================================
	//	This class contains mean and variance for each gradient pixel .
	//==================================================================
	class slGradientComp
	{
	public:
		slGradientComp();

		bool pixIsForground(const slPixel3ch &pixelX, const slPixel3ch &pixelY, float avgStdDev[3]) const;

		void add(const slPixel3ch &pixelX , const slPixel3ch &pixelY);

		inline float getVarianceB() const { return mgVarianceB; }
		inline float getVarianceG() const {	return mgVarianceG;	}
		inline float getVarianceR() const { return mgVarianceR; }

		static void setAlpha(const float alpha);
		static void setCoef(const float coef);

	private:
		float mNbPix;

		float mgxMeanB; 
		float mgxMeanG;
		float mgxMeanR;

		float mgyMeanB;
		float mgyMeanG;
		float mgyMeanR;

		float mgxVarianceB;
		float mgxVarianceG; 
		float mgxVarianceR;

		float mgyVarianceB;
		float mgyVarianceG; 
		float mgyVarianceR;

		float mgVarianceB;
		float mgVarianceG; 
		float mgVarianceR;

		//Learning rate parameter
		static float mAlpha;

		//Coefficient for Standard deviation gradient
	    static float mCoefGrad;

	};

private:
	bool mDoSobel;
	int mApSize;

	// Learning rate parameter
    float mAlpha;

	// Camera variance parameter for...
	float mSigmaIn;		// intensity
	float mSigmaCh;		// chromacity

	// Coefficient for Standard deviation
	float mCoeffInt;	// intensity
	float mCoeffChr;	// chromacity
	float mCoeffGrad;	// gradient

    // Mean and variance for...
	slIntensityComp* mIntPixels;	// intensity
	slChromacityComp* mChrPixels;	// chromacity
	slGradientComp* mGradPixels;	// gradient

	slImage3ch mGradX;	// gradient image for X
	slImage3ch mGradY;	// gradient image for Y

};


class SLBGSUB_DLL_EXPORT slSimpleGaussFactory: public slBgSubFactory
{
public:
	virtual ~slSimpleGaussFactory();

protected:
	// To specify your functions's parameters
	virtual void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const;

	// The factory (a static instance) will create an instance of your BgSubtractor
	virtual slSimpleGauss* createInstance() const;

private:
	// The factory's constructor
	slSimpleGaussFactory();

private:
	static slSimpleGaussFactory factory_;

};


#endif	// _SLSIMPLEGAUSS_H_


