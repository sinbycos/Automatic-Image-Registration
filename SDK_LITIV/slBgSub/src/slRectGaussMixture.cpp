/*!	\file	slRectGaussMixture.cpp
 *	\brief	Remake of rafik background subtractor with a mix of the
 *			GaussianMixture bgs
 *
 *	\author		Michael Sills Lavoie, (GaussMixture) Pier-Luc St-Onge
 *	\date		30.05.2007 - February 2012
 */


#include "slException.h"
#include "slRectGaussMixture.h"


using namespace cv;
using namespace slAH;


/*
*    Add the specific parameters to the ardHandler
*	 @param pParamSpecMap (pParamSpecMap) - The specific parameters for this bg Subtractor
*/
void slRectGaussMixture::fillParamSpecs(slAH::slParamSpecMap& paramSpecMap)
{
	slRectSimple::fillParamSpecs(paramSpecMap);
	slSpherGaussMixMat::fillParamSpecs(paramSpecMap);
}


slRectGaussMixture::slRectGaussMixture()
: slRectSimple()
{
}


/*
*    Destructor of the slRectSimple class.
*	 Delete every slRectPixels created in the class for each level.
*/
slRectGaussMixture::~slRectGaussMixture()
{
}


/*
*    Set parameters
*/
void slRectGaussMixture::setSubParameters(const slAH::slParameters& parameters)
{
	// Make sure the HSV color space is not used
	if (parameters.isParsed(ARG_COLOR_S) && strcmp(BGR_NAME, parameters.getValue(ARG_COLOR_S).c_str()) != 0) {
		throw slExceptionBgSub("slRectGaussMixture: Gaussian mixture only works in the rgb color space.");
	}

	// All Rect parameters
	slRectSimple::setSubParameters(parameters);

	// All gaussian mixture parameters
	gaussMixtures_.setParameters(parameters);
}


/*
*    Show the specific parameters for this Bg subtractor
*/
void slRectGaussMixture::showSubParameters() const
{
	slRectSimple::showSubParameters();
	gaussMixtures_.showParameters();
}


/*
*    Do the specific initialisation for this bgs
*	 We put everything that have to be done before the subtraction begin.
*    @param	  pArgHandlerBgSub (slArgHandler) -Width of the image in pixels
*	 @return  (int) - 0 (everything is ok)
*/
void slRectGaussMixture::init()
{
	/*************************************************************************
	*Initialisation for the gaussian mixture
	*************************************************************************/
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

	/*************************************************************************
	*Create the statistic of the frame
	*************************************************************************/

	slRectSimple::init();
}


/*
*    Here the computation of the current frame is done.
*	 This method is called once per frame in a video.
*/
void slRectGaussMixture::doSubtraction(slImage1ch &bForeground)
{
	const int w = imageSize_.width;
	const int h = imageSize_.height;

	// Empty foreground image
	bForeground = PIXEL_1CH_BLACK;

	//Update the foreground histogram (scan only the upper level, scan is recursive)
	#pragma omp parallel for
	for (int i = 0; i < (int)level_.back().size(); ++i)
	{
		level_.back()[i]->update(qCurrent_,qBackground_);
	}

	//Test the histogram (we test only the highest level, scan is recursive)
	//The test put a flag on the slRectPixels that are not background
	for (size_t i = 0; i < level_.back().size(); ++i)
	{
		level_.back()[i]->compareRectangle(th_,deltath_,Tth_,Tdeltath_);
	}

	//We check each one of the smalest slRectPixels to see if they are background
	#pragma omp parallel for
	for (int j = 0; j < statistic_.front().numberOfRectangleY; ++j)
	{
		for (int i = 0; i < statistic_.front().numberOfRectangleX ; ++i)
		{
			//If the small slRectPixels is in the background
			//We copy every pixel contained within it in the background
			if (level_.front()[j*statistic_.front().numberOfRectangleX + i]->getBackground())
			{
				int coordY = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getCoordY();
				int coordX = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getCoordX();
				int dimY = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getDimY();
				int dimX = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getDimX();

				for(int l = coordY; l < coordY+dimY; ++l)
				{
					const slPixel3ch* cur_row = current_[l];
					slPixel3ch* bg_row = background_[l];
					slPixel1ch* b_fg_row = bForeground[l];

					for(int k = coordX; k < coordX+dimX; ++k)
					{
						//Here are the pixel that the slRectPixels algorithm thinks are in the background

						// Update the gaussian Mixture
						gaussMixtures_.update(w*l+k, cur_row[k]);

						// Update background pixel
						bg_row[k] = cur_row[k];

						// Update binary foreground - non foreground pixel
						b_fg_row[k] = PIXEL_1CH_BLACK;
					}
				}
			}
			else
			{
				int coordY = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getCoordY();
				int coordX = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getCoordX();
				int dimY = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getDimY();
				int dimX = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getDimX();

				for(int l = coordY; l < coordY+dimY; ++l)
				{
					const slPixel3ch* cur_row = current_[l];
					slPixel3ch* bg_row = background_[l];
					slPixel1ch* b_fg_row = bForeground[l];

					for(int k = coordX; k < coordX+dimX; ++k)
					{
						// Evaluate if the quantified pixel is in the background with the GaussMixtures algo
						bool isBackground = gaussMixtures_.update(w*l+k, cur_row[k]);

						// If it's a background pixel
						if (isBackground)
						{
							// Update background pixel
							bg_row[k] = cur_row[k];

							// Update binary foreground - non foreground pixel
							b_fg_row[k] = PIXEL_1CH_BLACK;
						}
						// Foreground pixel
						else
						{
							// Update background pixel
							bg_row[k] = gaussMixtures_.getMean(w*l+k);

							// Update binary foreground - foreground pixel
							b_fg_row[k] = PIXEL_1CH_WHITE;
						}
					}
				}
			}
		}
	}

	//We clear the data stored in the slRectPixels for the next iteration
	#pragma omp parallel for
	for (int i = 0; i < (int)level_.back().size(); ++i)
	{
		level_.back()[i]->clearData();
	}

}


/*
*    Set a specific pixel in the background.
*	 This method is used by the filter that can be applied by the base bgs.
*	 @example If the shadow filter detect that a pixel is not foreground but background,
*			  it call this function to put it in the background
*    @param	  bg_data (slPixel3ch *) -The image of the background
*    @param	  fg_data (slPixel3ch *) -The image of the foreground
*    @param	  b_fg_data (slPixel3ch *) -The image of the binary foreground
*    @param	  w (int) -The width of the image
*    @param	  i (int) -The X coord of the pixel
*    @param	  j (int) -The Y coord of the pixel
*/
void slRectGaussMixture::setBgPixel(const slPixel3ch *cur_row,
	slPixel3ch *bg_row, slPixel1ch *b_fg_row, int w, int i, int j)
{
	// Update background pixel
	bg_row[j] = cur_row[j];

	// Update binary foreground - non foreground pixel
	b_fg_row[j] = PIXEL_1CH_BLACK;
}


/*
*    Do the specific things that need to be done when loading the next frame.
*	 @example	If the bgs needs another kind of image (ex.previous frame),
*				you quantifie it here.
*/
void slRectGaussMixture::prepareNextSubtraction()
{
}


void slRectGaussMixture::updateSubWindows()
{
}


///////////////////////////////////////////////////////////////////////////////
//	slRectGaussMixtureFactory
///////////////////////////////////////////////////////////////////////////////


slRectGaussMixtureFactory slRectGaussMixtureFactory::factory_;


slRectGaussMixtureFactory::slRectGaussMixtureFactory()
: slBgSubFactory("rectGaussMixture")
{
}


slRectGaussMixtureFactory::~slRectGaussMixtureFactory()
{
}


void slRectGaussMixtureFactory::fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const
{
	slRectGaussMixture::fillParamSpecs(paramSpecMap);
}


slRectGaussMixture* slRectGaussMixtureFactory::createInstance() const
{
	return new slRectGaussMixture();
}


