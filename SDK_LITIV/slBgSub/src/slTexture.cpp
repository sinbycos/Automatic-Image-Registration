/******************************************************************************
	File:	slSimpleGauss.cpp
	Description:	This is the Gaussian Mixture algorithm class.
	Created:	November 2006 (Pier-Luc St-Onge, Atousa Torabi, Parisa Darvish Zadeh Varcheie)
					Some comments (Pier-Luc St-Onge, Atousa Torabi, Parisa Darvish Zadeh Varcheie)
******************************************************************************/
#include "slTexture.h"


#define _USE_MATH_DEFINES


#include <algorithm>
#include <iostream>
#include <math.h>


using namespace std;


#define ARG_A		"-A"
#define ARG_NEIGHB	"-P"
#define ARG_RADIUS	"-R"
#define ARG_REGRAD	"-Rr"
#define ARG_K		"-K"
#define ARG_TB		"-Tb"
#define ARG_TP		"-Tp"
#define ARG_ALPHAB	"-Ab"
#define ARG_ALPHAW	"-Aw"


// Define the creator
//slTexture slTexture::Creator_ = slTexture("texture");


int slTexture::slHistogram::mP;
double slTexture::slHistogram::mAb;
double slTexture::slHistogram::mAw;

int slTexture::slTextureComp::mK;
double slTexture::slTextureComp::mTb;
double slTexture::slTextureComp::mTp;


slTexture::slTexture(const std::string &name):
slBgSub(name), mTexturePix(NULL), mCurrentHist(NULL)
{
	offsets[0] = offsets[1] = NULL;
}


slTexture::slTexture(const slParameters& parameters):
slBgSub(parameters), mTexturePix(NULL), mCurrentHist(NULL)
{
	// Threshold a
   mA = atoi(parameters.getValue(ARG_A).c_str());

	// Neighborhood size
	int P = atoi(parameters.getValue(ARG_NEIGHB).c_str());
	slHistogram::setP(P);

	// Neighborhood radius
	mR = atof(parameters.getValue(ARG_RADIUS).c_str());

	// Region radius
	mRr = atof(parameters.getValue(ARG_REGRAD).c_str());

	// Nb histograms
	slTextureComp::setK(atoi(parameters.getValue(ARG_K).c_str()));

	// Sum(w_i) T_B
	double Tb = atof(parameters.getValue(ARG_TB).c_str());
	slTextureComp::setTb(Tb);

	// Intersection T_P
	double Tp = atof(parameters.getValue(ARG_TP).c_str());
	slTextureComp::setTp(Tp);

	// Alpha b
	double Ab = atof(parameters.getValue(ARG_ALPHAB).c_str());
	slHistogram::setAb(Ab);

	// Alpha w
	double Aw = atof(parameters.getValue(ARG_ALPHAW).c_str());
	slHistogram::setAw(Aw);

	offsets[0] = new double[slHistogram::getP()];
	offsets[1] = new double[slHistogram::getP()];

	//find the place of neighbors computed form center pixel 
	for (int indA = 0; indA < P; indA++) {
		offsets[0][indA] = mR * cos(2 * M_PI * indA / P);
		offsets[1][indA] = mR * sin(2 * M_PI * indA / P);
	}
}


slTexture::~slTexture()
{
	if (mTexturePix != NULL) delete[] mTexturePix;
	if (mCurrentHist != NULL) delete[] mCurrentHist;
	if (offsets[0] != NULL) delete[] offsets[0];
	if (offsets[1] != NULL) delete[] offsets[1];
}


void slTexture::showSpecificParameters() const
{
	cout << "Threshold a : " << mA << endl;
    cout << "Neighborhood size : " << slHistogram::getP() << endl;
    cout << "Neighborhood radius : " << mR << endl;
    cout << "Region radius : " << mRr << endl;
	cout << "Nb histograms : " << slTextureComp::getK() << endl;
	cout << "Sum(w_i) T_B : " << slTextureComp::getTb() << endl;
	cout << "Intersection T_P : " << slTextureComp::getTp() << endl;
	cout << "Alpha b : " << slHistogram::getAb() << endl;
	cout << "Alpha w : " << slHistogram::getAw() << endl;

	cout << endl;
}


void slTexture::fillSpecificParameterSpecs(slParamSpecMap& paramSpecMap) const
{
	paramSpecMap
		<< (slParamSpec(ARG_A,		"Threshold a")			<< slSyntax("0..15",	"3"))
		<< (slParamSpec(ARG_NEIGHB,	"Neighborhood size")	<< slSyntax("3..8",	"6"))
		<< (slParamSpec(ARG_RADIUS,	"Neighborhood radius")	<< slSyntax("1..9",	"2"))
		<< (slParamSpec(ARG_REGRAD,	"Region radius")		<< slSyntax("2..15",	"9"))
		<< (slParamSpec(ARG_K,		"Nb histograms")		<< slSyntax("3..5",	"3"))
		<< (slParamSpec(ARG_TB,		"Sum(w_i) T_B")			<< slSyntax("0..1",	"0.4"))
		<< (slParamSpec(ARG_TP,		"Intersection T_P")		<< slSyntax("0..1",	"0.65"))
		<< (slParamSpec(ARG_ALPHAB,	"Alpha b")				<< slSyntax("0..1",	"0.01"))
		<< (slParamSpec(ARG_ALPHAW,	"Alpha w")				<< slSyntax("0..1",	"0.01"));
}


slBgSub* slTexture::createSpecificInstance(const slParameters& parameters) const
{
	if (parameters.isParsed("-c") && strcmp(parameters.getValue("-c").c_str(), BGR_NAME) != 0) {
		throw string("Texture only works in the rgb color space.");
	}
	
	return new slTexture(parameters);
}


int slTexture::specificInit()
{
	const int w = Size_.width;
	const int h = Size_.height;

	mTexturePix = new slTextureComp[h * w];
	mCurrentHist = new slHistogram[h * w];

	// Compute histograms for the initial background image
	computeTheHistograms(*imageList_[ARG_BG]);

	// For each row
	for (int i = 0; i < h; i++)
	{
		// For each column
		for (int j = 0; j < w; j++)
		{
			mTexturePix[w*i+j].setInitHistogram(mCurrentHist[w*i+j]);
		}
	}

	specificLoadNextFrame();

	return 0;
}


void slTexture::computeFrame()
{
	const int w = Size_.width;
	const int h = Size_.height;
    
	slPixel3ch* bg_data = imageList_[ARG_BG]->getPixel3chData();
	slPixel3ch* fg_data = imageList_[ARG_FG]->getPixel3chData();
	slPixel3ch* cur_data = imageList_[ARG_QC_FR]->getPixel3chData();
	slPixel1ch* b_fg_data = imageList_[ARG_BFG]->getPixel1chData();

	// For each row
	for (int i = 0; i < h; i++)
	{
		// For each column
		for (int j = 0; j < w; j++)
		{
			// If it's a background pixel
			if (mTexturePix[w*i+j].isPixBackground(mCurrentHist[w*i+j]))
			{
				// Update background pixel
				bg_data[w*i+j] = cur_data[w*i+j];

				// Update foreground pixel value
				fg_data[w*i+j] = (colorSystem_ == BGR ? BGR_WHITE : HSV_WHITE);

				// Update binary foreground - non foreground pixel
				b_fg_data[w*i+j] = PIXEL_1CH_BLACK;
			}
			// Foreground pixel
			else
			{
				// Update foreground pixel value
				fg_data[w*i+j] = cur_data[w*i+j];

				// Update binary foreground - foreground pixel
				b_fg_data[w*i+j] = PIXEL_1CH_WHITE;
			}
		}
	}
}


void slTexture::setBgPixel(slPixel3ch *bg_data, slPixel3ch *fg_data,
									 slPixel1ch *b_fg_data, const int& w, const int& i, const int& j)
{
	// Update background pixel
	bg_data[w*i+j] = fg_data[w*i+j];

	// Update foreground pixel value
	fg_data[w*i+j] = (colorSystem_ == BGR ? BGR_WHITE : HSV_WHITE);

	// Update binary foreground - non foreground pixel
	b_fg_data[w*i+j] = PIXEL_1CH_BLACK;
}


int slTexture::specificLoadNextFrame()
{
	computeTheHistograms(*imageList_[ARG_QC_FR]);

	return 0;
}


void slTexture::computeTheHistograms(const slImage &image)
{
	slImage grayImage = image.getGrayClone();
	const int w = Size_.width;
	const int h = Size_.height;
	const int P = slHistogram::getP();
	const slPixel1ch *data = grayImage.getConst1chData();
	unsigned char* lbpMatrix = new unsigned char[h * w];

	//--------------------------------
	// For each pixel, compute the LBP

	// For each row
	for (int i = 0; i < h; i++)
	{
		// For each column
		for (int j = 0; j < w; j++)
		{
			unsigned char lbp = 0;

			for (int p = 0; p < P; p++) {
				// Compute Gp using bilinear interpolation
				double x = j + offsets[0][p];
				double y = i + offsets[1][p];

				if (x < 0) x = 0;
				if (x > w - 1) x = w - 1;
				if (y < 0) y = 0;
				if (y > h - 1) y = h - 1;

				int fX = (int)floor(x);
				int cX = (int)ceil(x);
				int fY = (int)floor(y);
				int cY = (int)ceil(y);

				double alphaX = x - fX;
				double alphaY = y - fY;

				slPixel1ch pix00 = data[w * fY + fX];
				slPixel1ch pix01 = data[w * fY + cX];
				slPixel1ch pix10 = data[w * cY + fX];
				slPixel1ch pix11 = data[w * cY + cX];

				double Gp = (1 - alphaY) *					// Ligne 0
								(	(1 - alphaX) * pix00 +	// Colonne 0
									alphaX * pix01) +		// Colonne 1
							alphaY *						// Ligne 1
								(	(1 - alphaX) * pix10 +	// Colonne 0
									alphaX * pix11);		// Colonne 1

				lbp = (lbp << 1) | ((int)Gp - data[w*i+j] + mA >= 0 ? 1 : 0);
			}

			lbpMatrix[w*i+j] = lbp;
		}
	}

	//-------------------------------------------------------
	// For each pixel, compute the histograms in mCurrentHist

	int cRr = (int)ceil(mRr);

	// For each row
	for (int i = 0; i < h; i++)
	{
		// For each column
		for (int j = 0; j < w; j++)
		{
			slHistogram histTmp;

			// For each row in a sub-square
			for (int deltaI = -cRr; deltaI <= cRr; deltaI++) {
				int y = i + deltaI;

				if (0 <= y && y < h) {
					// For each column in a sub-square
					for (int deltaJ = -cRr; deltaJ <= cRr; deltaJ++) {
						int x = j + deltaJ;

						if (0 <= x && x < w) {
							// if the offset is still in the region of radius mRr
							if (deltaJ * deltaJ + deltaI * deltaI <= mRr * mRr) {
								// Add the lbp to the histogram
								histTmp.addLBP(lbpMatrix[w*y+x]);
							}
						}
					}
				}
			}

			mCurrentHist[w*i+j] = histTmp;
		}
	}

	delete[] lbpMatrix;
}


bool slTexture::slTextureComp::isPixBackground(const slHistogram &histogram)
{
	bool isBackground = false;
	double maxIntersection = 0;
	int indMax = 0;

	//----------------------------------------------
	// Find out if histogram is in the background
	// Find the histogram that fit it best

	for (int ind = 0; ind < mB; ind++)
	{
		double intersection = mHistograms[ind].intersection(histogram);

		// If ind is in the first B histograms and intersection is greater than Tp
		// The histogram is in the background
		if (intersection > mTp) isBackground = true;

		// Find the best of mHistograms if histogram is in the background
		if (isBackground && intersection > maxIntersection)
		{
			maxIntersection = intersection;
			indMax = ind;
		}
	}

	if (isBackground)
	{
		for (int ind = mB; ind < mK; ind++)
		{
			double intersection = mHistograms[ind].intersection(histogram);

			// Find the best of mHistograms if histogram is in the background
			if (intersection > maxIntersection)
			{
				maxIntersection = intersection;
				indMax = ind;
			}
		}
	}

	//----------------------
	// Update all histograms

	// If histogram is in the background
	if (isBackground)
	{
		// Update the histogram that fits "histogram"
		mHistograms[indMax] += histogram;

		// Update all weights (no need to normalize here)
		for (int ind = 0; ind < mK; ind++) {
			mHistograms[ind].addW(ind == indMax ? 1.0f : 0.0f);
		}
	}
	else	// If in the foreground
	{
		// Replace the last histogram with the current one
		mHistograms.back() = histogram;
		mHistograms.back().setW(0.01);

		// Normalize the weights
		double totalW = 0;

		// Sum all weights
		for (int ind = 0; ind < mK; ind++) {
			totalW += mHistograms[ind].getW();
		}

		// Do the normalisation
		for (int ind = 0; ind < mK; ind++) {
			mHistograms[ind].setW(mHistograms[ind].getW() / totalW);
		}
	}

	//--------------------
	// Sort all histograms

	sort(mHistograms.begin(), mHistograms.end());

	// Update mB
	double sumW = 0;
	mB = 0;

	while (sumW <= mTb && mB < mK) {
		sumW += mHistograms[mB].getW();
		mB++;
	}

	return isBackground;
}


double slTexture::slHistogram::intersection(const slHistogram& histogram)
{
	int sum = 0;
	int totalSum = 0;

	for (int ind = 0; ind < (1 << mP); ind++) {
		sum += min(mBins[ind], histogram.mBins[ind]);
		totalSum += mBins[ind];
	}

	return ((double)sum / totalSum);
}


