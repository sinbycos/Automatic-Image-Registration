#include <slException.h>

#include "slIn.h"


using namespace cv;


slIn::slIn()
{
	resetSpecs();
}


slIn::~slIn()
{
}


void slIn::resetSpecs()
{
	width_ = 0;
	height_ = 0;
	fps_ = 1;
	nbImages_ = 0;
}


double slIn::getFPS() const
{
	return fps_;
}


int slIn::getHeight() const
{
	return height_;
}


unsigned int slIn::getNbImages() const
{
	return nbImages_;
}


Size slIn::getSize() const
{
	return Size(width_, height_);
}


int slIn::getWidth() const
{
	return width_;
}


