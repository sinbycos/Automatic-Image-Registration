#include "slTempAvg.h"

#include <iostream>
#include <omp.h>


using namespace std;
using namespace slAH;


#define ARG_EPSILON	"-e"


slTempAvg::slTempAvg()
: slBgSub(), meanPix_(NULL)
{
	setEpsilon(15);
}


slTempAvg::~slTempAvg()
{
	if (meanPix_ != NULL) {
		delete [] meanPix_;
	}
}


void slTempAvg::fillParamSpecs(slParamSpecMap& paramSpecMap)
{
	paramSpecMap << (slParamSpec(ARG_EPSILON, "Epsilon") << slSyntax("0..255", "15"));
}


void slTempAvg::setEpsilon(int eps)
{
	// Epsilon
	if (colorSystem_ == SL_BGR) {
		epsilon_.setEpsilon(eps, eps, eps);
	}
	else {
		epsilon_.setEpsilon(eps * Q_H_LIMIT / Q_CHAN_LIMIT, eps, eps);
	}
}


void slTempAvg::setSubParameters(const slParameters& parameters)
{
	setEpsilon(atoi(parameters.getValue(ARG_EPSILON).c_str()));
}


void slTempAvg::showSubParameters() const
{
	cout << "--- slTempAvg ---" << endl;
	cout << "Epsilon : " << epsilon_ << endl;
}


void slTempAvg::init()
{
	if (meanPix_ != NULL) {
		delete [] meanPix_;
	}

	meanPix_ = new MeanPixel3ch[imageSize_.width * imageSize_.height]();
}


void slTempAvg::doSubtraction(slImage1ch &bForeground)
{
	const int w = imageSize_.width;
	const int h = imageSize_.height;

	// Empty foreground images
	bForeground = PIXEL_1CH_BLACK;

#pragma omp parallel for
	for (int i = 0; i < h; i++) {
		const slPixel3ch* cur_row = current_[i];
		const slPixel3ch* q_cur_row = qCurrent_[i];

		slPixel3ch* bg_row = background_[i];
		const slPixel3ch* q_bg_row = qBackground_[i];

		slPixel1ch* b_fg_row = bForeground[i];
		MeanPixel3ch* mean_row = &meanPix_[w * i];

		for (int j = 0; j < w; j++) {
			// Background pixel
			if (epsilon_.pixIsBackground(q_cur_row[j], q_bg_row[j], colorSystem_, doConsiderLightChanges_)) {
				// Update background pixel
				bg_row[j] = (mean_row[j] += cur_row[j]).getMeanPixel();
			}
			// Foreground pixel
			else {
				// Update binary foreground - foreground pixel
				b_fg_row[j] = PIXEL_1CH_WHITE;
			}
		}
	}
}


void slTempAvg::setBgPixel(const slPixel3ch *cur_row,
	slPixel3ch *bg_row, slPixel1ch *b_fg_row, int w, int i, int j)
{
	// Update background pixel
	bg_row[j] = (meanPix_[w*i+j] += cur_row[j]).getMeanPixel();

	// Update binary foreground - non foreground pixel
	b_fg_row[j] = PIXEL_1CH_BLACK;
}


void slTempAvg::prepareNextSubtraction()
{
}


void slTempAvg::updateSubWindows()
{
}


///////////////////////////////////////////////////////////////////////////////
//	slTempAvg::MeanPixel3ch
///////////////////////////////////////////////////////////////////////////////


slTempAvg::MeanPixel3ch::MeanPixel3ch()
: sumB_(0), sumG_(0), sumR_(0), nbPix_(0)
{
}


const slTempAvg::MeanPixel3ch& slTempAvg::MeanPixel3ch::operator+=(const slPixel3ch &pixel)
{
	sumB_ += pixel.val[0];
	sumG_ += pixel.val[1];
	sumR_ += pixel.val[2];

	nbPix_++;

	return *this;
}


slPixel3ch slTempAvg::MeanPixel3ch::getMeanPixel() const
{
	if (nbPix_ > 0)
		return slPixel3ch(	(sumB_ / nbPix_),
							(sumG_ / nbPix_),
							(sumR_ / nbPix_));
	else
		return slPixel3ch();
}


///////////////////////////////////////////////////////////////////////////////
//	slTempAvgFactory
///////////////////////////////////////////////////////////////////////////////


slTempAvgFactory slTempAvgFactory::factory_;


slTempAvgFactory::slTempAvgFactory()
: slBgSubFactory("tempAVG")
{
}


slTempAvgFactory::~slTempAvgFactory()
{
}


void slTempAvgFactory::fillParamSpecs(slParamSpecMap& paramSpecMap) const
{
	slTempAvg::fillParamSpecs(paramSpecMap);
}


slTempAvg* slTempAvgFactory::createInstance() const
{
	return new slTempAvg();
}


