/*
 *	  @file	   slNonFgMergeFg.cpp
 *    
 *	  @author  Michael Sills Lavoie 
 *      
 *    @date    24.05.2007
 */

#include "slNonFgMergeFg.h"

#include <iostream>
#include <omp.h>


using namespace std;


#define ARG_EPSILON	"-e"
#define ARG_NB_PIXEL_EPSILON "-ep"

#define ARG_QSBG	"-qsbg"
#define ARG_SBG		"-sbg"
#define ARG_QP_FR	"-qpf"
#define ARG_P_FR	"-pf"



slNonFgMergeFg slNonFgMergeFg::Creator_ = slNonFgMergeFg("nfgMerge");


slNonFgMergeFg::slNonFgMergeFg(const std::string &name):
slBgSub(name)
{
}


slNonFgMergeFg::slNonFgMergeFg(const slParameters& parameters):
slBgSub(parameters)
{
	imageList_[ARG_QSBG	] = new slImage();
	imageList_[ARG_QP_FR] = new slImage();
	imageList_[ARG_P_FR] = new slImage();
	imageList_[ARG_SBG] = new slImage();

	int eps = atoi(parameters.getValue(ARG_EPSILON).c_str());

	// Epsilon
	if (colorSystem_ == BGR)
		mEpsilon.setBGREpsilon(eps, eps, eps);
	else
		mEpsilon.setHSVEpsilon(eps * MAX_H / MAX_V, eps, eps);

	//NbPixel
	mNbPixel = atoi(parameters.getValue(ARG_NB_PIXEL_EPSILON).c_str());
}


slNonFgMergeFg::~slNonFgMergeFg()
{
}


void slNonFgMergeFg::showSpecificParameters() const
{
	cout << "Epsilon : " << mEpsilon << endl;
	cout << "Nb pixel to be static : " << mNbPixel << endl;

	cout << endl;
}


void slNonFgMergeFg::fillSpecificParameterSpecs(slParamSpecMap& paramSpecMap) const
{
	paramSpecMap << (slParamSpec(ARG_EPSILON, "Epsilon") << slSyntax("0..255", "15"));
	paramSpecMap << (slParamSpec(ARG_NB_PIXEL_EPSILON, "Nb of changing pixel to be considered static")
		<< slSyntax("{ 0..20000 }", "100"));
	paramSpecMap << slParamSpec(ARG_QSBG, "Quantified static background");
	paramSpecMap << slParamSpec(ARG_QP_FR, "Quantified previous frame");
	paramSpecMap << slParamSpec(ARG_SBG, "Static background");
	paramSpecMap << slParamSpec(ARG_P_FR, "Previous frame");
}


slBgSub* slNonFgMergeFg::createSpecificInstance(const slParameters& parameters) const
{
	return new slNonFgMergeFg(parameters);
}


int slNonFgMergeFg::specificInit()
{
	imageList_[ARG_QSBG	]->cloneImage(*imageList_[ARG_QBG	]);
	imageList_[ARG_SBG	]->cloneImage(*imageList_[ARG_BG	]);
	imageList_[ARG_QP_FR]->cloneImage(*imageList_[ARG_QC_FR	]);
	imageList_[ARG_P_FR]->cloneImage(*imageList_[ARG_C_FR	]);

	mNbPixelCount = 0;

	return 0;
}


void slNonFgMergeFg::computeFrame()
{
	const int w = Size_.width;
	const int h = Size_.height;

	slPixel3ch* bg_data = imageList_[ARG_BG]->getPixel3chData();
	slPixel3ch* fg_data = imageList_[ARG_FG]->getPixel3chData();

	slPixel3ch* q_bg_data = imageList_[ARG_QBG]->getPixel3chData();
	slPixel1ch* b_fg_data = imageList_[ARG_BFG]->getPixel1chData();

	slPixel3ch* cur_data = imageList_[ARG_C_FR]->getPixel3chData();
	slPixel3ch* q_cur_data = imageList_[ARG_QC_FR]->getPixel3chData();

	slPixel3ch* qp_data = imageList_[ARG_QP_FR]->getPixel3chData();
	slPixel3ch* p_data = imageList_[ARG_P_FR]->getPixel3chData();

	slPixel3ch* q_sbg_data = imageList_[ARG_QSBG]->getPixel3chData();
	slPixel3ch* sbg_data = imageList_[ARG_SBG]->getPixel3chData();

	bool updateStaticBg;

	//Check if the number of changed pixel in the previous frame 
	//is superior to epsilon
	if (mNbPixelCount > mNbPixel)
		updateStaticBg = false;
	else
		updateStaticBg = true;

	//Reset the changed foreground pixel in a frame
	mNbPixelCount = 0;

//#pragma omp parallel for
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			// Update dynamic background
			if (mEpsilon.pixIsBackground(q_cur_data[w*i+j], qp_data[w*i+j],
				colorSystem_, doConsiderLightChanges_))
			{
				// dynamic background pixel
				bg_data[w*i+j] = cur_data[w*i+j];

				// Update foreground pixel value
				fg_data[w*i+j] = (colorSystem_ == BGR ? BGR_WHITE : HSV_WHITE);

				// Update binary foreground - non foreground pixel
				b_fg_data[w*i+j] = PIXEL_1CH_BLACK;

				if(updateStaticBg)
					sbg_data[w*i+j] = cur_data[w*i+j];
			}
			// If pixel in cur_frame is from background
			// add it to dynamic bg
			else
			{
				//Increase the nb of changed pixel by 1
				++mNbPixelCount;

				if (mEpsilon.pixIsBackground(q_cur_data[w*i+j], q_sbg_data[w*i+j],
				colorSystem_, doConsiderLightChanges_))
				{
					// dynamic background pixel
					bg_data[w*i+j] = cur_data[w*i+j];

					// Update foreground pixel value
					fg_data[w*i+j] = (colorSystem_ == BGR ? BGR_WHITE : HSV_WHITE);

					// Update binary foreground - non foreground pixel
					b_fg_data[w*i+j] = PIXEL_1CH_BLACK;
				}
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
	//update the previous frame
	imageList_[ARG_P_FR]->cloneImage(*imageList_[ARG_C_FR	]);

}


void slNonFgMergeFg::setBgPixel(slPixel3ch *bg_data, slPixel3ch *fg_data,
										  slPixel1ch *b_fg_data, const int& w, const int& i, const int& j)
{
	// Update background pixel
	bg_data[w*i+j] = fg_data[w*i+j];

	// Update foreground pixel value
	fg_data[w*i+j] = (colorSystem_ == BGR ? BGR_WHITE : HSV_WHITE);

	// Update binary foreground - non foreground pixel
	b_fg_data[w*i+j] = PIXEL_1CH_BLACK;

	--mNbPixelCount;
}


int slNonFgMergeFg::specificLoadNextFrame()
{
	if (doQuantification_)
	{
		imageList_[ARG_QP_FR]->quantify3ch(Quantification_, *imageList_[ARG_P_FR]);
		imageList_[ARG_QSBG	]->quantify3ch(Quantification_, *imageList_[ARG_SBG	]);
	}
	else
	{
		imageList_[ARG_QP_FR]->copyImage(*imageList_[ARG_P_FR	]);
		imageList_[ARG_QSBG	]->copyImage(*imageList_[ARG_SBG]);
	}
	return 0;
}


