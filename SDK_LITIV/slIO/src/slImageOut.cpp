#include "slImageOut.h"


using namespace cv;


slImageOut::slImageOut(const std::string &filename_format, int index)
: slOut()
{
	setFormat(filename_format);
	setIndex(index);
}


slImageOut::~slImageOut()
{
}


void slImageOut::close()
{
	setFormat("");
	setIndex(0);
}


slImageOut& slImageOut::open(const std::string &filename_format)
{
	setFormat(filename_format);
	return *this;
}


void slImageOut::write(const slMat &image)
{
	if (!format_.empty() && !image.empty()) {
		char filename[512];

		sprintf_s(filename, 512, format_.c_str(), index_++);
		filename[511] = 0;

		imwrite(filename, image);
	}
}


slImageOut& slImageOut::setFormat(const std::string &filename_format)
{
	format_ = filename_format;
	return *this;
}


slImageOut& slImageOut::setIndex(int index)
{
	index_ = index;
	return *this;
}


