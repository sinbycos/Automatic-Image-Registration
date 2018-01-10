/******************************************************************************
	File:	slGaussMixture.cpp
	Description:	This is the implementation of the
					Gaussian Mixture algorithm.
	Created:	October 2006 (Pier-Luc St-Onge)
	Modified:	November 2006:
					Some comments (Pier-Luc St-Onge)
******************************************************************************/

#include "slGaussMixture.h"

#include <algorithm>


using namespace cv;
using namespace slAH;


slGaussMixture::slGaussMixture()
: slBgSub()
{
}


slGaussMixture::~slGaussMixture()
{
}


void slGaussMixture::fillParamSpecs(slAH::slParamSpecMap& paramSpecMap)
{
	slSpherGaussMixMat::fillParamSpecs(paramSpecMap);
}


void slGaussMixture::setSubParameters(const slAH::slParameters& parameters)
{
	// Make sure the HSV color space is not used
	if (parameters.isParsed(ARG_COLOR_S) &&
		strcmp(parameters.getValue(ARG_COLOR_S).c_str(), BGR_NAME) != 0)
	{
		throw slException("Gaussian mixture only works in the rgb color space.");
	}

	// All gaussian mixture parameters
	gaussMixtures_.setParameters(parameters);
}


void slGaussMixture::showSubParameters() const
{
	gaussMixtures_.showParameters();
}


void slGaussMixture::init()
{
	if (colorSystem_ == SL_HSV) {
		throw slException("Gaussian mixture only works in the rgb color space.");
	}

	const int w = imageSize_.width;
	const int h = imageSize_.height;

	// Create a new gaussMixtures_ vector
	gaussMixtures_.reset(w * h);

	// For each row
#pragma omp parallel for
	for (int i = 0; i < h; i++) {
		const int wi = w * i;
		const slPixel3ch* bg_row = background_[i];

		// For each column
		for (int j = 0; j < w; j++) {
			// Initialize the gaussian mixture
			gaussMixtures_.update(wi + j, bg_row[j]);
		}
	}
}


void slGaussMixture::doSubtraction(slImage1ch &bForeground)
{
	const int w = imageSize_.width;
	const int h = imageSize_.height;

	// Empty foreground images
	bForeground = PIXEL_1CH_BLACK;

	// For each row
#pragma omp parallel for
	for (int i = 0; i < h; i++) {
		const int wi = w * i;
		const slPixel3ch* cur_row = current_[i];

		slPixel3ch* bg_row = background_[i];
		slPixel1ch* b_fg_row = bForeground[i];

		// For each column
		for (int j = 0; j < w; j++) {
			// Evaluate if the quantified pixel is in the background
			bool isBackground = gaussMixtures_.update(wi + j, cur_row[j]);

			// Update the background pixel
			bg_row[j] = gaussMixtures_.getMean(wi + j);

			// Foreground pixel
			if (!isBackground) {
				// Update binary foreground - foreground pixel
				b_fg_row[j] = PIXEL_1CH_WHITE;
			}
		}
	}
}


void slGaussMixture::setBgPixel(const slPixel3ch *cur_row,
		slPixel3ch *bg_row, slPixel1ch *b_fg_row, int w, int i, int j)
{
	// Update binary foreground - non foreground pixel
	b_fg_row[j] = PIXEL_1CH_BLACK;
}


void slGaussMixture::prepareNextSubtraction()
{
	// Nothing to do here
}


void slGaussMixture::updateSubWindows()
{
}


///////////////////////////////////////////////////////////////////////////////
//	slGaussMixtureFactory
///////////////////////////////////////////////////////////////////////////////


slGaussMixtureFactory slGaussMixtureFactory::factory_;


slGaussMixtureFactory::slGaussMixtureFactory()
: slBgSubFactory("gaussMixture")
{
}


slGaussMixtureFactory::~slGaussMixtureFactory()
{
}


void slGaussMixtureFactory::fillParamSpecs(slParamSpecMap& paramSpecMap) const
{
	slGaussMixture::fillParamSpecs(paramSpecMap);
}


slGaussMixture* slGaussMixtureFactory::createInstance() const
{
	return new slGaussMixture();
}


