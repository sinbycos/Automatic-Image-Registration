#include "slOpenCv.h"

#pragma warning(disable : 4996)
#include <cvaux.h>
#pragma warning(default : 4996)
#include <iostream>


using namespace std;


slOpenCv slOpenCv::Creator_ = slOpenCv("opencv");


slOpenCv::slOpenCv(const std::string &name):
slBgSub(name), mStatModel(NULL)
{
}


slOpenCv::slOpenCv(const slParameters& parameters):
slBgSub(parameters), mStatModel(NULL)
{
}


slOpenCv::~slOpenCv()
{
	if (mStatModel != NULL) cvReleaseBGStatModel(&mStatModel);
}


void slOpenCv::showSpecificParameters() const
{
	cout << endl;
}


void slOpenCv::fillSpecificParameterSpecs(slParamSpecMap& paramSpecMap) const
{
}


slBgSub* slOpenCv::createSpecificInstance(const slParameters& parameters) const
{
	return new slOpenCv(parameters);
}


int slOpenCv::specificInit()
{
   mStatModel = cvCreateFGDStatModel(imageList_[ARG_BG]->getIplImage());

	return 0;
}


void slOpenCv::computeFrame()
{
	cvUpdateBGStatModel(imageList_[ARG_QC_FR]->getIplImage(), mStatModel);

	cvCopy(mStatModel->background, imageList_[ARG_BG]->getIplImage());
	cvCopy(mStatModel->foreground, imageList_[ARG_BFG]->getIplImage());

	CvScalar couleur = (colorSystem_ == BGR ? CV_RGB(BGR_WHITE.r(), BGR_WHITE.g(), BGR_WHITE.b()) :
		CV_RGB(HSV_WHITE.h(), HSV_WHITE.s(), HSV_WHITE.v()));

	cvSet(imageList_[ARG_FG]->getIplImage(), couleur);
	cvCopy(imageList_[ARG_C_FR]->getIplImage(), imageList_[ARG_FG]->getIplImage(), mStatModel->foreground);
}


void slOpenCv::setBgPixel(slPixel3ch *bg_data, slPixel3ch *fg_data,
										 slPixel1ch *b_fg_data, const int& w, const int& i, const int& j)
{
	// Update foreground pixel value
	fg_data[w*i+j] = (colorSystem_ == BGR ? BGR_WHITE : HSV_WHITE);

	// Update binary foreground - non foreground pixel
	b_fg_data[w*i+j] = PIXEL_1CH_BLACK;
}


int slOpenCv::specificLoadNextFrame()
{
	return 0;
}


