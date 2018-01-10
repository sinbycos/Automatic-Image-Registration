/*
 *	  @file	   slNonFgKeepFg.h
 *    
 *	  @author  Michael Sills Lavoie 
 *      
 *    @date    24.05.2007
 */

#include "slNonFgKeepFg.h"

#include <iostream>
#include <omp.h>


using namespace std;


#define ARG_EPSILON	"-e"

slNonFgKeepFg slNonFgKeepFg::Creator_ = slNonFgKeepFg("nfgKeep");


slNonFgKeepFg::slNonFgKeepFg(const std::string &name):
slBgSub(name)
{
}


slNonFgKeepFg::slNonFgKeepFg(const slParameters& parameters):
slBgSub(parameters)
{
	int eps = atoi(parameters.getValue(ARG_EPSILON).c_str());

	// Epsilon
	if (colorSystem_ == BGR)
		mEpsilon.setBGREpsilon(eps, eps, eps);
	else
		mEpsilon.setHSVEpsilon(eps * MAX_H / MAX_V, eps, eps);
}


slNonFgKeepFg::~slNonFgKeepFg()
{
}


void slNonFgKeepFg::showSpecificParameters() const
{
	cout << "Epsilon : " << mEpsilon << endl;

	cout << endl;
}


void slNonFgKeepFg::fillSpecificParameterSpecs(slParamSpecMap& paramSpecMap) const
{
	paramSpecMap << (slParamSpec(ARG_EPSILON, "Epsilon") << slSyntax("0..255", "15"));
}


slBgSub* slNonFgKeepFg::createSpecificInstance(const slParameters& parameters) const
{
	return new slNonFgKeepFg(parameters);
}


int slNonFgKeepFg::specificInit()
{
	return 0;
}


void slNonFgKeepFg::computeFrame()
{
	const int w = Size_.width;
	const int h = Size_.height;

	slPixel3ch* bg_data = imageList_[ARG_BG]->getPixel3chData();
	slPixel3ch* fg_data = imageList_[ARG_FG]->getPixel3chData();

	slPixel3ch* q_bg_data = imageList_[ARG_QBG]->getPixel3chData();
	slPixel1ch* b_fg_data = imageList_[ARG_BFG]->getPixel1chData();

	slPixel3ch* cur_data = imageList_[ARG_C_FR]->getPixel3chData();
	slPixel3ch* q_cur_data = imageList_[ARG_QC_FR]->getPixel3chData();

#pragma omp parallel for
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			// Background pixel
			if (mEpsilon.pixIsBackground(q_cur_data[w*i+j], q_bg_data[w*i+j],
				colorSystem_, doConsiderLightChanges_))
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


void slNonFgKeepFg::setBgPixel(slPixel3ch *bg_data, slPixel3ch *fg_data,
										 slPixel1ch *b_fg_data, const int& w, const int& i, const int& j)
{
	// Update background pixel
	bg_data[w*i+j] = fg_data[w*i+j];

	// Update foreground pixel value
	fg_data[w*i+j] = (colorSystem_ == BGR ? BGR_WHITE : HSV_WHITE);

	// Update binary foreground - non foreground pixel
	b_fg_data[w*i+j] = PIXEL_1CH_BLACK;
}


int slNonFgKeepFg::specificLoadNextFrame()
{
	return 0;
}


