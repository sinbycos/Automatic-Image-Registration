#include "slVideoIn.h"


using namespace cv;
using namespace std;


slVideoIn::slVideoIn()
: slIn(), capture_(NULL)
{
}


slVideoIn::~slVideoIn()
{
	releaseCapture();
}


void slVideoIn::releaseCapture()
{
	delete capture_;
	capture_ = NULL;
}


void slVideoIn::close()
{
	releaseCapture();
	resetSpecs();
}


slVideoIn& slVideoIn::open(const std::string &name)
{
	VideoCapture *capture = new VideoCapture(name);

	if (capture == NULL || capture->isOpened() == false) {
		delete capture;
		throw slExceptionIO(string("slVideoIn::open(): cannot open \"") + name + "\"");
	}

	releaseCapture();
	capture_ = capture;

	width_ = (int)capture_->get(CV_CAP_PROP_FRAME_WIDTH);
	height_ = (int)capture_->get(CV_CAP_PROP_FRAME_HEIGHT);
	fps_ = capture_->get(CV_CAP_PROP_FPS);
	nbImages_ = (unsigned int)capture_->get(CV_CAP_PROP_FRAME_COUNT);

	return *this;
}


void slVideoIn::read(slMat &image)
{
	if (capture_ != NULL) {
		(*capture_) >> image;
	}
}


