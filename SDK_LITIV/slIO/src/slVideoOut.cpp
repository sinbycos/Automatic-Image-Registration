#include "slVideoOut.h"


using namespace cv;
using namespace std;


slVideoOut::slVideoOut()
: slOut(), fourcc_(-1), width_(0), height_(0), fps_(1), isColor_(true), writer_(NULL)
{
}


slVideoOut::~slVideoOut()
{
	releaseWriter();
}


void slVideoOut::releaseWriter()
{
	delete writer_;
	writer_ = NULL;
}


void slVideoOut::close()
{
	releaseWriter();
}


void slVideoOut::setCodec(int fourcc)
{
	fourcc_ = fourcc;
}


void slVideoOut::setFPS(double fps)
{
	fps_ = fps;
}


void slVideoOut::setIsColor(bool isColor)
{
	isColor_ = isColor;
}


void slVideoOut::setSize(cv::Size size)
{
	width_ = size.width;
	height_ = size.height;
}


slVideoOut& slVideoOut::open(const std::string &name)
{
	VideoWriter *writer = new VideoWriter(name, fourcc_, fps_, Size(width_, height_), isColor_);

	if (writer == NULL || writer->isOpened() == false) {
		delete writer;
		throw slExceptionIO(string("slVideoIn::open(): cannot open \"") + name + "\"");
	}

	releaseWriter();
	writer_ = writer;

	return *this;
}


void slVideoOut::write(const slMat &image)
{
	if (writer_ != NULL) {
		(*writer_) << image;
	}
}


