#include "slImageIn.h"


using namespace cv;
using namespace std;


slImageIn::slImageIn(const std::string &filename_format, int index)
: slIn()
{
	setFormat(filename_format);
	setIndex(index);
}


slImageIn::~slImageIn()
{
}


void slImageIn::close()
{
	setFormat("");
	setIndex(0);
	resetSpecs();
}


slImageIn& slImageIn::open(const std::string &filename_format)
{
	setFormat(filename_format);
	int index = index_;

	try {
		slMat image;
		read(image);

		index_ = index;
		width_ = image.cols;
		height_ = image.rows;
		fps_ = 1;
		nbImages_ = 1;	// TODO: Count numbered image files
	}
	catch (...) {
		index_ = index;
		resetSpecs();
		throw;
	}

	return *this;
}


void slImageIn::read(slMat &image)
{
	if (!format_.empty()) {
		char filename[512];

		sprintf_s(filename, 512, format_.c_str(), index_++);
		filename[511] = 0;

		image = imread(filename);

		if (image.empty()) {
			throw slExceptionIO(string("slImageIn::compute(): cannot open \"") + filename + "\"");
		}
	}
}


slImageIn& slImageIn::setFormat(const std::string &filename_format)
{
	format_ = filename_format;
	return *this;
}


slImageIn& slImageIn::setIndex(int index)
{
	index_ = index;
	return *this;
}


