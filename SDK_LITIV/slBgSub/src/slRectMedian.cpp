/*
 *	  @file	   slRectMedian.h
 *	  Remake of rafik background subtractor updated with
 *	  the median bgs
 *    
 *	  @author  Michael Sills Lavoie
 *	  @author (Median) ???
 *      
 *    @date    30.05.2007
 */

#include "slRectMedian.h"

#include <iostream>


#define ARG_BASE_EPSILON	"-re"
#define ARG_DELTA_EPSILON "-dre"
#define ARG_MIN_RECTANGLE	"-mr"
#define ARG_M_VALUE	"-m"
#define ARG_DIST_MPDA "-mpda"
#define ARG_TEXTURE "-tex"
#define ARG_TEXTURE_BASE_EPSILON "-rte"
#define ARG_TEXTURE_DELTA_EPSILON "-drte"

// Define the creator
//slRectMedian slRectMedian::Creator_ = slRectMedian("rectmedian");


slRectMedian::slRectMedian(const string &name):
slBgSub(name), mMedians(NULL)
{
}


slRectMedian::slRectMedian(const slParameters& parameters):
slBgSub(parameters), mMedians(NULL)
{
	//BASE EPSILON
	th_ =(float) atof(parameters.getValue(ARG_BASE_EPSILON).c_str());

	//DELTA EPSILON
	deltath_ = (float) atof(parameters.getValue(ARG_DELTA_EPSILON).c_str());

	//MINIMAL NUMBER OF slRectPixels IN THE UPPER LEVEL
	minRect_ = atoi(parameters.getValue(ARG_MIN_RECTANGLE).c_str());

	if (parameters.isParsed(ARG_DIST_MPDA))
	{
		slHistogram3ch::setDistanceMethod(true);
		MPDA_ = true;
	}
	else
	{
		slHistogram3ch::setDistanceMethod(false);
		MPDA_ = false;
	}

	//BASE EPSILON USED TO COMPARE THE UPPER LEVEL OR RECTANGLE TEXTURE
	Tth_ = (float)atof(parameters.getValue(ARG_TEXTURE_BASE_EPSILON).c_str());

	//DELTA EPSILON USED TO INCREASE THE TEXTURE EPSILON WHEN WE GO DOWN THE LEVEL
	Tdeltath_ = (float)atof(parameters.getValue(ARG_TEXTURE_DELTA_EPSILON).c_str());

	if (parameters.isParsed(ARG_TEXTURE))
	{
		slRectPixels::setTexture(true);
		useTexture_ = true;
	}
	else
	{
		slRectPixels::setTexture(false);
		useTexture_ = false;
	}

	//THE HISTOGRAM NEED TO KNOW WHAT IS THE COLOR SPACE
	if (parameters.isParsed("-c") && strcmp(parameters.getValue("-c").c_str(), HSV_NAME) == 0)
		isHSV_ = true;
	else
		isHSV_ = false;

	slHistogram3ch::setColorSpace(isHSV_);

	//Set the range of the histogram to considered the quantification
	slHistogram3ch::setRange(	atoi(parameters.getValue("-q", 0).c_str()),
								atoi(parameters.getValue("-q", 1).c_str()),
								atoi(parameters.getValue("-q", 2).c_str()));

	// M Value
	mMValue = atoi(parameters.getValue(ARG_M_VALUE).c_str());
}

/*
*    Destructor of the slRectSimple class.
*	 Delete every slRectPixels created in the class for each level.
*/
slRectMedian::~slRectMedian()
{
	for (size_t i = 0; i < statistic_.size(); ++i)
	{
		//Destruction of the Rectangles for each level...
		for(int j = 0; j < statistic_[i].numberOfRectangleY ; ++j)
		{
			for(int k = 0; k < statistic_[i].numberOfRectangleX; ++k)
			{
				delete level_[i][j*statistic_[i].numberOfRectangleX + k];
			}
		}
	}
}

/*
*    Show the specific parameters for this Bg subtractor
*/
void slRectMedian::showSpecificParameters() const
{
	cout << "M_Value : " << mMValue << endl;
	cout << "Base epsilon : " << th_ << endl;
	cout << "Delta epsilon : " << deltath_ << endl;
	cout << "Minimal number of slRectPixels : " << minRect_ <<endl;
	if (useTexture_)
		cout << "Use texture." << endl;
	else
		cout << "Don't use texture distance." << endl;
	cout << "Base epsilon for the texture of the upper rectangle : " << Tth_<<endl;
	cout << "Delta epsilon for the texture : " << Tdeltath_ << endl;
	if (MPDA_)
		cout << "Use MPDA distance." << endl;
	else
		cout << "Don't use MPDA distance." << endl;

	cout << endl;
}

/*
*    Add the specific parameters to the ardHandler
*	 @param pParamSpecMap (pParamSpecMap) - The specific parameters for this bg Subtractor
*/
void slRectMedian::fillSpecificParameterSpecs(slParamSpecMap& paramSpecMap) const
{
	paramSpecMap
		<< (slParamSpec(ARG_BASE_EPSILON,	"Base epsilon")						<< slSyntax("0.0..1.0",	"0.25"))
		<< (slParamSpec(ARG_DELTA_EPSILON,	"Delta epsilon")					<< slSyntax("0.0..1.0",	"0.15"))
		<< (slParamSpec(ARG_MIN_RECTANGLE,	"Minimal number of slRectPixels")	<< slSyntax("1..1024",	"25"))
		<< (slParamSpec(ARG_M_VALUE,		"M value")							<< slSyntax("1..256",	"5"))
		<< slParamSpec(ARG_DIST_MPDA, "Use the MPDA distance (suppose to be more precise")
		<< slParamSpec(ARG_TEXTURE, "Use the the texture to ajust the histogram th")
		<< (slParamSpec(ARG_TEXTURE_BASE_EPSILON,	"Base texture epsilon")		<< slSyntax("0.0..60.0", "1.0"))
		<< (slParamSpec(ARG_TEXTURE_DELTA_EPSILON,	"Texture delta epsilon")	<< slSyntax("-5.0..5.0", "0"));
}

/*
*    Create a apecific instance of this bg subtractor
*    @param	  pArgHandlerBgSub (slArgHandler) -Width of the image in pixels
*	 @return  (slBgSub*) -the created bgs
*/
slBgSub* slRectMedian::createSpecificInstance(const slParameters& parameters) const
{
	if (!parameters.isParsed("-q"))
		throw string("Color must be quantified.");

	// Make sure the HSV color space is not used with the texture
	if (parameters.isParsed(ARG_TEXTURE))
	{
		if (parameters.isParsed("-c") && strcmp(BGR_NAME, parameters.getValue("-c").c_str()) != 0) {
			throw string("Texture only works in the rgb color space.");
		}
	}

	return new slRectMedian(parameters);
}

/*
*    Do the specific initialisation for this bgs
*	 We put everything that have to be done before the subtraction begin.
*    @param	  pArgHandlerBgSub (slArgHandler) -Width of the image in pixels
*	 @return  (int) - 0 (everything is ok)
*/
int slRectMedian::specificInit()
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

	/*************************************************************************
	*Create the statistic of the frame
	*************************************************************************/

	//Determine the maximum number of division (number of level)
	//And set the statistic of each one
	int width = Size_.width;
	int height = Size_.height;
	float ratio;
	bool isOk = true;
	int numberOfLevel = 0;

	ratio = (float)width/height;

	//IN THE FUTURE, ADD OTHER SCREEN FORMAT
	//If the ratio is 4/3
	if((ratio - 1.33333333) <= 0.000001)
	{
		int RectangleWidth = 4;
		int RectangleHeight = 3;

		//Create every level (if the frame can be divided)
		while(isOk)
		{
			if (((width % RectangleWidth) == 0) && ((height % RectangleHeight) == 0) && 
				((((float)width / RectangleWidth) * ((float)height / RectangleHeight)) >= minRect_) )
			{
				statistic_.push_back(slLevelStat());
				statistic_[numberOfLevel].RectangleWidth = RectangleWidth;
				statistic_[numberOfLevel].RectangleHeight = RectangleHeight;
				statistic_[numberOfLevel].numberOfRectangleX = (int)((float)width / RectangleWidth);
				statistic_[numberOfLevel].numberOfRectangleY = (int)((float)height / RectangleHeight);

				RectangleWidth *= 2;
				RectangleHeight *= 2;
				numberOfLevel++;
			}
			else
				isOk = false;
		}
	}

	/************************************************************
	//Make sure that the screen format is recognize by the bgSub.
	*************************************************************/
	if(isOk)
		throw string("Image ratio isn't supported, please add it in the code or try resizing (-r w h)");

	cout << "Number of level in this image : " << statistic_.size() << endl;
	cout << "Number of slRectPixels on the upper level : " << 
	statistic_.back().numberOfRectangleX * statistic_.back().numberOfRectangleY <<endl;

	
	//Creation of each level of Rectangles for the foreground and foreground
	createLevel(level_);

	return 0;
}

/*
*    Here the computation of the current frame is done.
*	 This method is called once per frame in a video.
*/
void slRectMedian::computeFrame()
{
	const int w = Size_.width;
	const int h = Size_.height;

	//Background
	slPixel3ch* bg_data = imageList_[ARG_BG]->getPixel3chData();
	//Foreground
	slPixel3ch* fg_data = imageList_[ARG_FG]->getPixel3chData();

	//Quantified background
	slPixel3ch* q_bg_data = imageList_[ARG_QBG]->getPixel3chData();
	//Binary foreground
	slPixel1ch* b_fg_data = imageList_[ARG_BFG]->getPixel1chData();

	//Current frame
	slPixel3ch* cur_data = imageList_[ARG_C_FR]->getPixel3chData();
	//Quantified current frame
	slPixel3ch* q_cur_data = imageList_[ARG_QC_FR]->getPixel3chData();

	//Create the median values
	if (indFrame_ < mMValue)
	{
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
			{
				// Update background pixel
				bg_data[w*i+j] = (mMedians[w*i+j] += cur_data[w*i+j]).getMeanPixel();

				// Update foreground pixel value
				fg_data[w*i+j] = (colorSystem_ == BGR ? BGR_WHITE : HSV_WHITE);

				// Update binary foreground - non foreground pixel
				b_fg_data[w*i+j] = PIXEL_1CH_BLACK;
			}
		}
	}
	else
	{
		//Update the foreground histogram (scan only the upper level, scan is recursive)
		#pragma omp parallel for
		for (int i = 0; i < (int)level_.back().size(); ++i)
		{
			level_.back()[i]->update(q_cur_data,q_bg_data,w);
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
				//We copy every pixel contained within it
				if (level_.front()[j*statistic_.front().numberOfRectangleX + i]->getBackground())
				{
					int coordY = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getCoordY();
					int coordX = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getCoordX();
					int dimY = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getDimY();
					int dimX = level_.front()[j*statistic_.front().numberOfRectangleX + i]->getDimX();

					for(int l = coordY; l < coordY+dimY; ++l)
					{
						for(int k = coordX; k < coordX+dimX; ++k)
						{
							// Update background pixel
							bg_data[w*l+k] = (mMedians[w*l+k] += cur_data[w*l+k]).getMeanPixel();

							// Update foreground pixel value
							fg_data[w*l+k] = (colorSystem_ == BGR ? BGR_WHITE : HSV_WHITE);

							// Update binary foreground - non foreground pixel
							b_fg_data[w*l+k] = PIXEL_1CH_BLACK;
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
						for(int k = coordX; k < coordX+dimX; ++k)
						{
							// Update foreground pixel value
							fg_data[w*l+k] = cur_data[w*l+k];

							// Update binary foreground - foreground pixel
							b_fg_data[w*l+k] = PIXEL_1CH_WHITE;
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
void slRectMedian::setBgPixel(slPixel3ch *bg_data, slPixel3ch *fg_data,
									slPixel1ch *b_fg_data, const int& w, const int& i, const int& j)
{
	// Update background pixel
	bg_data[w*i+j] = (mMedians[w*i+j] += fg_data[w*i+j]).getMeanPixel();

	// Update foreground pixel value
	fg_data[w*i+j] = (colorSystem_ == BGR ? BGR_WHITE : HSV_WHITE);

	// Update binary foreground - non foreground pixel
	b_fg_data[w*i+j] = PIXEL_1CH_BLACK;
}

/*
*    Do the specific things that need to be done when loading the next frame.
*	 @example	If the bgs needs another kind of image (ex.previous frame),
*				you quantifie it here.
*/
int slRectMedian::specificLoadNextFrame()
{
	return 0;
}

/*
*    Create every level of slRectPixels and fill them with slRectPixels
*    @param	  level (vector<vector<slRectPixels*>> &) -The level that need to be filled
*/
/*void slRectMedian::createLevel (vector<vector<slRectPixels*>> &level)
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
							if((j*2 + g) >= 0 && (j*2 + g) < statistic_[i-1].numberOfRectangleY && (k*2 + h) >= 0 && ((k*2 + h) < statistic_[i-1].numberOfRectangleY))
								if(!((h == 0 && g ==0) || (h == 1 && g == 1) || (h == 0 && g == 1) || (h == 1 && g == 0)))
									level[i][j*statistic_[i].numberOfRectangleX + k]->addExternalLink(level[i-1][((j*2+g)*statistic_[i-1].numberOfRectangleX) + (k*2) + h]);
						}
					}
				}
			}
		}
	}
}*/

