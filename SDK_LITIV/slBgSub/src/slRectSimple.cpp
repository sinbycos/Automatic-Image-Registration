/*!	\file	slRectSimple.cpp
 *	\brief	Remake of rafik background subtractor.
 *
 *	\author		Michael Sills Lavoie, Pier-Luc St-Onge
 *	\date		24.05.2007 - February 2012
 */

#include "slException.h"
#include "slRectSimple.h"

#include <iostream>


using namespace std;
using namespace slAH;


#define ARG_BASE_EPSILON	"-re"
#define ARG_DELTA_EPSILON "-dre"
#define ARG_MIN_RECTANGLE	"-mr"
#define ARG_DIST_MPDA "-mpda"
#define ARG_TEXTURE "-tex"
#define ARG_TEXTURE_BASE_EPSILON "-rte"
#define ARG_TEXTURE_DELTA_EPSILON "-drte"


/*
*    Add the specific parameters to the ardHandler
*	 @param pParamSpecMap (pParamSpecMap) - The specific parameters for this bg Subtractor
*/
void slRectSimple::fillParamSpecs(slAH::slParamSpecMap& paramSpecMap)
{
	paramSpecMap
		<< (slParamSpec(ARG_BASE_EPSILON,	"Base epsilon")						<< slSyntax("0.0..1.0",	"0.25"))
		<< (slParamSpec(ARG_DELTA_EPSILON,	"Delta epsilon")					<< slSyntax("0.0..1.0",	"0.15"))
		<< (slParamSpec(ARG_MIN_RECTANGLE,	"Minimal number of slRectPixels")	<< slSyntax("1..1024",	"25"))
		<< slParamSpec(ARG_DIST_MPDA, "Use the MPDA distance (suppose to be more precise)")
		<< slParamSpec(ARG_TEXTURE, "Use the the texture to ajust the histogram th")
		<< (slParamSpec(ARG_TEXTURE_BASE_EPSILON, "Base texture epsilon")		<< slSyntax("0.0..60.0", "1.0"))
		<< (slParamSpec(ARG_TEXTURE_DELTA_EPSILON, "Texture delta epsilon")		<< slSyntax("-5.0..5.0", "0"));
}


slRectSimple::slRectSimple()
: slBgSub()
{
	setBaseEps(0.25f);
	setDeltaEps(0.15f);

	setMinRect(25);

	setTextureBaseEps(1.0f);
	setTextureDeltaEps(0.f);

	setTexture(false);
	setDistanceMethod(false);

	setColorSystem(SL_BGR);
	setQuantification(true);
}


/*
*    Destructor of the slRectSimple class.
*	 Delete every slRectPixels created in the class for each level.
*/
slRectSimple::~slRectSimple()
{
	for (size_t i = 0; i < statistic_.size(); ++i)
	{
		//Destruction of the Rectangles for each level...
		for (int j = 0; j < statistic_[i].numberOfRectangleY ; ++j)
		{
			for (int k = 0; k < statistic_[i].numberOfRectangleX; ++k)
			{
				delete level_[i][j*statistic_[i].numberOfRectangleX + k];
			}
		}
	}
}


void slRectSimple::setBaseEps(float th)
{
	th_ = th;
}


void slRectSimple::setDeltaEps(float deltath)
{
	deltath_ = deltath;
}


void slRectSimple::setMinRect(int minRect)
{
	minRect_ = minRect;
}


void slRectSimple::setTextureBaseEps(float Tth)
{
	Tth_ = Tth;
}


void slRectSimple::setTextureDeltaEps(float Tdeltath)
{
	Tdeltath_ = Tdeltath;
}


void slRectSimple::setTexture(bool useTexture)
{
	slRectPixels::setTexture(useTexture);
	useTexture_ = useTexture;
}


void slRectSimple::setDistanceMethod(bool mpda)
{
	slHistogram3ch::setDistanceMethod(mpda);
	MPDA_ = mpda;
}


/*!
 *	Sets the slHistogram3ch color space: slHistogram3ch::setColorSpace().
 */
void slRectSimple::setColorSystem(typeColorSys colorSystem)
{
	slBgSub::setColorSystem(colorSystem);
	slHistogram3ch::setColorSpace(colorSystem == SL_HSV);
}


/*!
 *	Sets the slHistogram3ch range: slHistogram3ch::setRange().
 */
void slRectSimple::setQuantification(bool enabled, const slQuant3ch& quant)
{
	if (!enabled) {
		throw slExceptionBgSub("slRectSimple::setQuantification(): Color must be quantified");
	}

	slBgSub::setQuantification(enabled, quant);
	slHistogram3ch::setRange(	quant.getQ0().getNbLevels(),
								quant.getQ1().getNbLevels(),
								quant.getQ2().getNbLevels());
}


/*
*    Create a apecific instance of this bg subtractor
*    @param	  pArgHandlerBgSub (slArgHandler) -Width of the image in pixels
*	 @return  (slBgSub*) -the created bgs
*/
void slRectSimple::setSubParameters(const slAH::slParameters& parameters)
{
	//We check if the color is quantified (it must be)
	if (!parameters.isParsed(ARG_QUANT))
		throw slExceptionBgSub("slRectSimple: Color must be quantified.");

	// Make sure the HSV color space is not used with the texture
	if (parameters.isParsed(ARG_TEXTURE))
	{
		if (parameters.isParsed(ARG_COLOR_S) && strcmp(BGR_NAME, parameters.getValue(ARG_COLOR_S).c_str()) != 0) {
			throw slExceptionBgSub("slRectSimple: Texture only works in the rgb color space.");
		}
	}

	//BASE EPSILON USED TO COMPARE THE UPPER LEVEL OR slRectPixels HISTOGRAM
	setBaseEps((float)atof(parameters.getValue(ARG_BASE_EPSILON).c_str()));

	//DELTA EPSILON USED TO INCREASE EPSILON WHEN WE GO DOWN THE LEVEL
	setDeltaEps((float)atof(parameters.getValue(ARG_DELTA_EPSILON).c_str()));

	//MINIMAL NUMBER OF slRectPixels IN THE UPPER LEVEL
	setMinRect(atoi(parameters.getValue(ARG_MIN_RECTANGLE).c_str()));

	//BASE EPSILON USED TO COMPARE THE UPPER LEVEL OR RECTANGLE TEXTURE
	setTextureBaseEps((float)atof(parameters.getValue(ARG_TEXTURE_BASE_EPSILON).c_str()));

	//DELTA EPSILON USED TO INCREASE THE TEXTURE EPSILON WHEN WE GO DOWN THE LEVEL
	setTextureDeltaEps((float)atof(parameters.getValue(ARG_TEXTURE_DELTA_EPSILON).c_str()));

	//TEXTURE USED OR NOT
	setTexture(parameters.isParsed(ARG_TEXTURE));

	//MDPA distance for the histograms
	setDistanceMethod(parameters.isParsed(ARG_DIST_MPDA));
}


/*
*    Show the specific parameters for this Bg subtractor
*/
void slRectSimple::showSubParameters() const
{
	cout << "--- slRectSimple ---" << endl;
	cout << "Base epsilon for the upper slRectPixels : " << th_ << endl;
	cout << "Delta epsilon : " << deltath_ << endl;
	cout << "Minimal number of slRectPixels in the upper level : " << minRect_ << endl;
	if (useTexture_)
		cout << "Use texture." << endl;
	else
		cout << "Don't use texture distance." << endl;
	cout << "Base epsilon for the texture of the upper rectangle : " << Tth_ << endl;
	cout << "Delta epsilon for the texture : " << Tdeltath_ << endl;
	if (MPDA_)
		cout << "Use MPDA distance." << endl;
	else
		cout << "Don't use MPDA distance." << endl;
}


/*
*    Do the specific initialisation for this bgs
*	 We put everything that have to be done before the subtraction begin.
*    @param	  pArgHandlerBgSub (slArgHandler) -Width of the image in pixels
*	 @return  (int) - 0 (everything is ok)
*/
void slRectSimple::init()
{
	/*************************************************************************
	*Create the statistic of the frames 
	*************************************************************************/

	//Determine the maximum number of division (number of level)
	//And set the statistic of each one
	int width = imageSize_.width;
	int height = imageSize_.height;
	int gcd = width, mod = height;
	slLevelStat statistic;

	// Greatest common divisor, Euclid's algorithm
	while (mod > 0) {
		int nextGCD = mod;
		mod = gcd % mod;
		gcd = nextGCD;
	}

	// Clear all statistics
	statistic_.clear();

	// Smallest rectangles statistics
	statistic.RectangleWidth = width / gcd;
	statistic.RectangleHeight = height / gcd;
	statistic.numberOfRectangleX = gcd;
	statistic.numberOfRectangleY = gcd;

	// Create every level (if the frame can be divided)
	while (	statistic.numberOfRectangleX * statistic.RectangleWidth == width &&
			statistic.numberOfRectangleY * statistic.RectangleHeight == height && 
			statistic.numberOfRectangleX * statistic.numberOfRectangleY >= minRect_)
	{
		statistic_.push_back(statistic);

		statistic.RectangleWidth <<= 1;		// *= 2
		statistic.RectangleHeight <<= 1;	// *= 2
		statistic.numberOfRectangleX >>= 1;	// /= 2
		statistic.numberOfRectangleY >>= 1;	// /= 2
	}

	/************************************************************
	//Make sure that the screen format is recognize by the bgSub.
	*************************************************************/
	if (statistic_.empty()) {
		throw slExceptionBgSub("Image ratio isn't supported, please add it in the code or try resizing (-r w h)");
	}

	cout << "Number of level in this image : " << statistic_.size() << endl;
	cout << "Number of slRectPixels on the upper level : " <<
		statistic_.back().numberOfRectangleX * statistic_.back().numberOfRectangleY <<endl;

	//Creation of each level of Rectangles for the foreground and foreground
	createLevel(level_);
}


/*
*    Here the computation of the current frame is done.
*	 This method is called once per frame in a video.
*/
void slRectSimple::doSubtraction(slImage1ch &bForeground)
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
						// Update background pixel
						bg_row[k] = cur_row[k];

						// Update binary foreground - non foreground pixel
						b_fg_row[k] = PIXEL_1CH_BLACK;
					}
				}
			}
			//Every pixel are copied in the foreground
			else
			{
				int coordY = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getCoordY();
				int coordX = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getCoordX();
				int dimY = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getDimY();
				int dimX = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getDimX();

				for(int l = coordY; l < coordY+dimY; ++l)
				{
					slPixel1ch* b_fg_row = bForeground[l];

					for(int k = coordX; k < coordX+dimX; ++k)
					{
						// Update binary foreground - foreground pixel
						b_fg_row[k] = PIXEL_1CH_WHITE;
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
void slRectSimple::setBgPixel(const slPixel3ch *cur_row,
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
void slRectSimple::prepareNextSubtraction()
{
}


void slRectSimple::updateSubWindows()
{
}


/*
*    Create every level of slRectPixels and fill them with slRectPixels
*    @param	  level (vector<vector<slRectPixels*>> &) -The level that need to be filled
*/
void slRectSimple::createLevel (std::vector<std::vector<slRectPixels*> > &level)
{
	for (size_t i = 0; i < statistic_.size(); ++i)
	{
		level.push_back(vector<slRectPixels*>());
		level[i].reserve(statistic_[i].numberOfRectangleX * statistic_[i].numberOfRectangleY);

		//Creation of the Rectangles for each level...
		for(int j = 0; j < statistic_[i].numberOfRectangleY ; ++j)
		{
			for(int k = 0; k < statistic_[i].numberOfRectangleX; ++k)
			{
				//For the smalest slRectPixels, we put a tag  "true" to say that they are at the base level (0)
				//The two lasts two arguments are used to know what pixel belong to who 
				if(i == 0)
					level[i].push_back(new slRectPixels(k*statistic_[i].RectangleWidth,j*statistic_[i].RectangleHeight,
					true,statistic_[i].RectangleWidth,statistic_[i].RectangleHeight));
				else
				{
					level[i].push_back(new slRectPixels(k*statistic_[i].RectangleWidth,j*statistic_[i].RectangleHeight,
					false,statistic_[i].RectangleWidth,statistic_[i].RectangleHeight));
					
					//We add pointers to the slRectPixels contained by each slRectPixels of superior level 4 Rectangles per slRectPixels
					level[i][j*statistic_[i].numberOfRectangleX + k]->addLink(level[i-1][(j*statistic_[i-1].numberOfRectangleX)*2 + k*2],
						level[i-1][(j*statistic_[i-1].numberOfRectangleX)*2 + (k*2)+1],
						level[i-1][((j*2+1)*statistic_[i-1].numberOfRectangleX) + (k*2)],
						level[i-1][((j*2+1)*statistic_[i-1].numberOfRectangleX)+ (k*2)+1]);
					
					//We then add links to the external slRectPixels bording each superior slRectPixels (12 or less in the corners)
					for(int g = -1; g < 3; ++g)
					{
						for(int h = -1; h < 3; ++h)
						{
							if((j*2 + g) >= 0 && (j*2 + g) < statistic_[i-1].numberOfRectangleY && (k*2 + h) >= 0 &&((k*2 + h) < statistic_[i-1].numberOfRectangleY))
								if(!((h == 0 && g ==0) || (h == 1 && g == 1) || (h == 0 && g == 1) || (h == 1 && g == 0)))
									level[i][j*statistic_[i].numberOfRectangleX + k]->addExternalLink(level[i-1][((j*2+g)*statistic_[i-1].numberOfRectangleX) + (k*2) + h]);
						}
					}
				}
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
//	slRectSimpleFactory
///////////////////////////////////////////////////////////////////////////////


slRectSimpleFactory slRectSimpleFactory::factory_;


slRectSimpleFactory::slRectSimpleFactory()
: slBgSubFactory("rect")
{
}


slRectSimpleFactory::~slRectSimpleFactory()
{
}


void slRectSimpleFactory::fillParamSpecs(slAH::slParamSpecMap& paramSpecMap) const
{
	slRectSimple::fillParamSpecs(paramSpecMap);
}


slRectSimple* slRectSimpleFactory::createInstance() const
{
	return new slRectSimple();
}


