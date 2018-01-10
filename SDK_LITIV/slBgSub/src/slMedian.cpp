#include "slMedian.h"

#include <iostream>


#define ARG_M_VALUE	"-m"
#define ARG_EPSILON	"-e"


slMedian slMedian::Creator_ = slMedian("median");


slMedian::slMedian(const string &name):
slBgSub(name), mMedians(NULL)
{
}


slMedian::slMedian(const slParameters& parameters):
slBgSub(parameters), mMedians(NULL)
{
	// M Value
	mMValue = atoi(parameters.getValue(ARG_M_VALUE).c_str());

	int eps = atoi(parameters.getValue(ARG_EPSILON).c_str());

	// Epsilon
	if (colorSystem_ == BGR)
		mEpsilon.setBGREpsilon(eps, eps, eps);
	else
		mEpsilon.setHSVEpsilon(eps * MAX_H / MAX_V, eps, eps);
}


slMedian::~slMedian()
{
}


void slMedian::showSpecificParameters() const
{
	cout << "M_Value : " << mMValue << endl;
	cout << "Epsilon : " << mEpsilon << endl;

	cout << endl;
}


void slMedian::fillSpecificParameterSpecs(slParamSpecMap& paramSpecMap) const
{
	paramSpecMap << (slParamSpec(ARG_M_VALUE, "M value") << slSyntax("1..256", "20"));
	paramSpecMap << (slParamSpec(ARG_EPSILON, "Epsilon") << slSyntax("0..255", "15"));
}


slBgSub* slMedian::createSpecificInstance(const slParameters& parameters) const
{
	return new slMedian(parameters);
}


int slMedian::specificInit()
{
	mMedians.resize(Size_.width * Size_.height);

	// Resize all slMedianPixel3ch objects to mMValue elements
	for (int i = 0; i < Size_.height; i++)
	{
		for (int j = 0; j < Size_.width; j++)
		{
			mMedians[Size_.width * i + j].resize(mMValue);
		}
	}

	return 0;
}


void slMedian::computeFrame()
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
			if (indFrame_ < mMValue || mEpsilon.pixIsBackground(
				q_cur_data[w*i+j], q_bg_data[w*i+j], colorSystem_, doConsiderLightChanges_))
			{
				// Update background pixel
				bg_data[w*i+j] = (mMedians[w*i+j] += cur_data[w*i+j]).getMeanPixel();

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


void slMedian::setBgPixel(slPixel3ch *bg_data, slPixel3ch *fg_data,
									slPixel1ch *b_fg_data, const int& w, const int& i, const int& j)
{
	// Update background pixel
	bg_data[w*i+j] = (mMedians[w*i+j] += fg_data[w*i+j]).getMeanPixel();

	// Update foreground pixel value
	fg_data[w*i+j] = (colorSystem_ == BGR ? BGR_WHITE : HSV_WHITE);

	// Update binary foreground - non foreground pixel
	b_fg_data[w*i+j] = PIXEL_1CH_BLACK;
}


int slMedian::specificLoadNextFrame()
{
	return 0;
}


